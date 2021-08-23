/*
*   Compressor Module
*       by Jacob Curtis
*
*   Originally inspired by Daniel Rudrich's "Simple Compressor"
*   https://github.com/DanielRudrich/SimpleCompressor
* 
*   Envelope implementaiton from:
*   https://christianfloisand.wordpress.com/2014/06/09/dynamics-processing-compressorlimiter-part-1/
*
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

class Compressor
{
public:
    void setParameters(const juce::AudioProcessorValueTreeState& apvts)
    {
        threshold = apvts.getRawParameterValue("threshold")->load();
        const float attackInput = apvts.getRawParameterValue("attack")->load();
        attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * static_cast<float>(sampleRate)));
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * static_cast<float>(sampleRate)));
        const float ratio = apvts.getRawParameterValue("ratio")->load();
        slope = 1.0f - (1.0f / ratio);
        makeUpGain = apvts.getRawParameterValue("makeUp")->load();
        scFreq = apvts.getRawParameterValue("scFreq")->load();
        scBypass = apvts.getRawParameterValue("scBypass")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        const float mixInput = apvts.getRawParameterValue("mix")->load();
        mix = mixInput / 100.0f;
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        dryBuffer.setSize(numOutputs, maxBlockSize);
        sideChainBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // copy the buffer into the dry buffer and sidechain
        for (int channel = 0; channel < numOutputs; channel++)
        {
            dryBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
            sideChainBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
        }
        // apply sidechain filter if not bypassed
        if (!scBypass)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                filters[channel].setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, scFreq));
                filters[channel].processSamples(sideChainBuffer.getWritePointer(channel), bufferSize);
            }
        }
        createEnvelope();
        applyCompression();
        // copy compressed signal to output
        for (int channel = 0; channel < numOutputs; channel++)
        {
            inputBuffer.copyFrom(channel, 0, sideChainBuffer.getReadPointer(channel), bufferSize);
        }
    }

    float getGainReductionLeft()
    {
        return (outputGainReduction[0] * -1.0f);
    }

    float getGainReductionRight()
    {
        return (outputGainReduction[1] * -1.0f);
    }

private:
    // use sidechain buffer to compute envelope buffer
    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (stereo)
            {
                const float maxSample = juce::jmax(std::abs(sideChainBuffer.getSample(0, sample)),
                    std::abs(sideChainBuffer.getSample(1, sample)));
                // apply attack
                if (compressionLevel[0] < maxSample)
                {
                    compressionLevel[0] = maxSample + attackTime * (compressionLevel[0] - maxSample);
                }
                // apply release
                else
                {
                    compressionLevel[0] = maxSample + releaseTime * (compressionLevel[0] - maxSample);
                }
                // write envelope
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    envelopeBuffer.setSample(channel, sample, compressionLevel[0]);
                }
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    const float inputSample = std::abs(sideChainBuffer.getSample(channel, sample));
                    // apply attack
                    if (compressionLevel[channel] < inputSample)
                    {
                        compressionLevel[channel] = inputSample + attackTime * (compressionLevel[channel] - inputSample);
                    }
                    // apply release
                    else
                    {
                        compressionLevel[channel] = inputSample + releaseTime * (compressionLevel[channel] - inputSample);
                    }
                    // write envelope
                    envelopeBuffer.setSample(channel, sample, compressionLevel[channel]);
                }
            }
        }
    }

    // use envelope buffer to compress sidechain buffer and mix with dry buffer
    void applyCompression()
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        // convert mix to sin6dB
        const float dryMix = std::pow(std::sin(0.5f * juce::float_Pi * (1.0f - mix)), 2.0f);
        const float wetMix = std::pow(std::sin(0.5f * juce::float_Pi * mix), 2.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // apply threshold and ratio to envelope
                float currenGainReduction = slope * (threshold - juce::Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample)));
                // remove positive gain reduction
                currenGainReduction = juce::jmin(0.0f, currenGainReduction);
                // set meter values
                if (currenGainReduction < outputGainReduction[channel])
                {
                    outputGainReduction[channel] = currenGainReduction;
                }
                // add makeup gain and convert decibels to gain
                currenGainReduction = std::pow(10.0f, 0.05f * (currenGainReduction + makeUpGain));
                // compute mix
                const float wetSample = dryBuffer.getSample(channel, sample) * currenGainReduction * wetMix;
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                // sum wet and dry to sidechain
                sideChainBuffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float threshold{ -10.0f };
    float attackTime{ 10.0f };
    float releaseTime{ 50.0f };
    float slope{ 0.75f };
    float makeUpGain{ 0.0f };
    float scFreq{ 20.0f };
    bool scBypass{ true };
    bool stereo{ true };
    float mix{ 100.0f };
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    juce::AudioBuffer<float> dryBuffer, sideChainBuffer, envelopeBuffer;
    std::array<juce::IIRFilter, numOutputs> filters;
};