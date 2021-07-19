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

class Compressor
{
public:
    Compressor(){}

    ~Compressor() {}

    void updateCompressorValues(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values
        threshold = apvts.getRawParameterValue("threshold")->load();
        float attackInput = apvts.getRawParameterValue("attack")->load();
        float releaseInput = apvts.getRawParameterValue("release")->load();
        float ratio = apvts.getRawParameterValue("ratio")->load();
        makeUpGain = apvts.getRawParameterValue("makeUp")->load();
        scFreq = apvts.getRawParameterValue("scFreq")->load();
        scBypass = apvts.getRawParameterValue("scBypass")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        float mixInput = apvts.getRawParameterValue("mix")->load();
        // calculate values
        attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * (float)sampleRate));
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
        slope = 1.0f - (1.0f / ratio);
        mix = mixInput / 100.0f;
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        // initialize buffers
        dryBuffer.setSize(numChannels, maxBlockSize);
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        envelopeBuffer.setSize(numChannels, maxBlockSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context, double inputSampleRate)
    {
        const auto& outputBuffer = context.getOutputBlock();
        // copy the buffer into the dry buffer and sidechain
        for (int channel = 0; channel < 2; channel++)
        {
            juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
            juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
        }
        // apply sidechain filter if not bypassed
        if (!scBypass)
        {
            auto hpfCoefficients = juce::IIRCoefficients::makeHighPass(inputSampleRate, scFreq);
            for (int channel = 0; channel < 2; channel++)
            {
                filters[channel].setCoefficients(hpfCoefficients);
                float* data = sideChainBuffer.getWritePointer(channel);
                filters[channel].processSamples(data, bufferSize);
            }
        }
        // create envelope and apply gain reduction to sidechain
        createEnvelope();
        calculateGainReduction();
        // copy compressed signal to output
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), sideChainBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), sideChainBuffer.getReadPointer(1), bufferSize);
    }

    // use sidechain buffer to compute envelope buffer
    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (stereo)
            {
                // find max of input samples
                const float maxSample = juce::jmax(std::abs(sideChainBuffer.getSample(0, sample)), std::abs(sideChainBuffer.getSample(1, sample)));
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
                for (int channel = 0; channel < 2; channel++)
                {
                    envelopeBuffer.setSample(channel, sample, compressionLevel[0]);
                }
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                for (int channel = 0; channel < 2; channel++)
                {
                    // get absolute value of sample
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
    void calculateGainReduction()
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        // convert mix to sin6dB
        float dryMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (1.0 - mix)), 2.0));
        float wetMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * mix), 2.0));
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < 2; channel++)
            {
                // apply threshold and ratio to envelope
                float currenGainReduction = slope * (threshold - juce::Decibels::gainToDecibels(envelopeBuffer.getSample(channel, sample)));
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
                float wetSample = dryBuffer.getSample(channel, sample) * currenGainReduction * wetMix;
                float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                // sum wet and dry to sidechain
                sideChainBuffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    const float getGainReductionLeft()
    {
        return (outputGainReduction[0] * -1.0f);
    }

    const float getGainReductionRight()
    {
        return (outputGainReduction[1] * -1.0f);
    }

private:
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
    float compressionLevel[2]{ 0.0f, 0.0f };
    float outputGainReduction[2]{ 0.0f, 0.0f };
    juce::AudioBuffer<float> dryBuffer, sideChainBuffer, envelopeBuffer;
    juce::IIRFilter filters[2];
};