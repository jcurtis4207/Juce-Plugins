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

#define numOutputs 2

class Limiter
{
public:
    void setParameters(const juce::AudioProcessorValueTreeState& apvts)
    {
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        releaseTime = static_cast<float>(std::exp(-1.0f / (releaseInput * sampleRate / 1000.0)));
        stereo = apvts.getRawParameterValue("stereo")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        sideChainBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // copy the buffer into the sidechain
        for (int channel = 0; channel < numOutputs; channel++)
        {
            sideChainBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
        }
        createEnvelope();
        calculateGainReduction();
        // apply gain reduction to output
        for (int channel = 0; channel < numOutputs; channel++)
        {
            juce::FloatVectorOperations::multiply(inputBuffer.getWritePointer(channel), 
                sideChainBuffer.getReadPointer(channel), bufferSize);
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
    // use sidechain to create compression envelope
    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (stereo)
            {
                const float maxSample = juce::jmax(std::abs(sideChainBuffer.getSample(0, sample)),
                    std::abs(sideChainBuffer.getSample(1, sample)));
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
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    const float inputSample = std::abs(sideChainBuffer.getSample(channel, sample));
                    // apply instant attack
                    if (compressionLevel[channel] < inputSample)
                    {
                        compressionLevel[channel] = inputSample;
                    }
                    // apply release
                    else
                    {
                        compressionLevel[channel] = inputSample + releaseTime *
                            (compressionLevel[channel] - inputSample);
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
        outputGainReduction = { 0.0f, 0.0f };
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // apply threshold to envelope
                float currentGainReduction = threshold - juce::Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample));
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

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    float releaseTime{ 1.0f };
    bool stereo{ true };
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    juce::AudioBuffer<float> sideChainBuffer, envelopeBuffer;
};