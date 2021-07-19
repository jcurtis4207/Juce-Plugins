/*
*   Limiter Module (adapted from Compression module)
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

class Limiter
{
public:
    Limiter(){}

    ~Limiter() {}

    void updateLimiterValues(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
        float releaseInput = apvts.getRawParameterValue("release")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        // calculate release time
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
    }

    void prepare(const double inputSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        envelopeBuffer.setSize(numChannels, maxBlockSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        // copy the buffer into the sidechain
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // create envelope and calculate compression multiplier
        createEnvelope();
        calculateGainReduction();
        // apply gain reduction to output
        juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(0), sideChainBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(1), sideChainBuffer.getReadPointer(1), bufferSize);
    }

    // use sidechain to create compression envelope
    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (stereo)
            {
                // find max of input samples
                const float maxSample = juce::jmax(std::abs(sideChainBuffer.getSample(0, sample)), std::abs(sideChainBuffer.getSample(1, sample)));
                // apply instant attack
                if (compressionLevel[0] < maxSample)
                {
                    compressionLevel[0] = maxSample;
                }
                // apply release
                else
                {
                    compressionLevel[0] = maxSample + releaseTime * (compressionLevel[0] - maxSample);
                }
                // write envelope
                envelopeBuffer.setSample(0, sample, compressionLevel[0]);
                envelopeBuffer.setSample(1, sample, compressionLevel[0]);
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                for (int channel = 0; channel < 2; channel++)
                {
                    // get absolute value of sample
                    const float inputSample = std::abs(sideChainBuffer.getSample(channel, sample));
                    // apply instant attack
                    if (compressionLevel[channel] < inputSample)
                    {
                        compressionLevel[channel] = inputSample;
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

    // calculate compression multiplier from envelope to sidechain
    void calculateGainReduction()
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < 2; channel++)
            {
                // apply threshold to envelope
                float currentGainReduction = threshold - juce::Decibels::gainToDecibels(envelopeBuffer.getSample(channel, sample));
                // remove positive gain reduction
                currentGainReduction = juce::jmin(0.0f, currentGainReduction);
                // set gr meter values
                if (currentGainReduction < outputGainReduction[channel])
                {
                    outputGainReduction[channel] = currentGainReduction;
                }
                // convert decibels to gain, add autogain, and add ceiling
                currentGainReduction = std::pow(10.0f, 0.05f * (currentGainReduction - threshold + ceiling));
                // output compression multiplier to sidechain
                sideChainBuffer.setSample(channel, sample, currentGainReduction);
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
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    float releaseTime{ 1.0f };
    bool stereo{ true };
    float compressionLevel[2]{ 0.0f, 0.0f };
    float outputGainReduction[2]{ 0.0f, 0.0f };
    juce::AudioBuffer<float> sideChainBuffer, envelopeBuffer;
};