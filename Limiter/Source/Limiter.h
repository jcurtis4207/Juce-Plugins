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

using namespace juce;

struct Parameters {
    float threshold, ceiling, releaseTime;
    bool stereo;
};

class Limiter
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.threshold = apvts.getRawParameterValue("threshold")->load();
        parameters.ceiling = apvts.getRawParameterValue("ceiling")->load();
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        parameters.releaseTime = static_cast<float>(std::exp(-1.0f / (releaseInput * sampleRate / 1000.0)));
        parameters.stereo = apvts.getRawParameterValue("stereo")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        compressionBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        compressionBuffer.makeCopyOf(inputBuffer, true);
        createEnvelope();
        calculateGainReduction();
        applyLimiting(inputBuffer);
    }

    std::array<float, numOutputs> getGainReduction()
    {
        return std::array<float, numOutputs>{
            outputGainReduction[0] * -1.0f,
            outputGainReduction[1] * -1.0f
        };
    }

private:

    void applyHisteresis(float& compLevel, float inputSample)
    {
        float releaseLevel = inputSample + parameters.releaseTime * (compLevel - inputSample);
        compLevel = (compLevel < inputSample) ? inputSample : releaseLevel;
    }

    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            if (parameters.stereo)
            {
                const float maxSample = jmax(std::abs(compressionBuffer.getSample(0, sample)),
                    std::abs(compressionBuffer.getSample(1, sample)));
                applyHisteresis(compressionLevel[0], maxSample);
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    envelopeBuffer.setSample(channel, sample, compressionLevel[0]);
                }
            }
            else
            {
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    const float inputSample = std::abs(compressionBuffer.getSample(channel, sample));
                    applyHisteresis(compressionLevel[channel], inputSample);
                    envelopeBuffer.setSample(channel, sample, compressionLevel[channel]);
                }
            }
        }
    }

    void calculateGainReduction()
    {
        outputGainReduction = { 0.0f, 0.0f };
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // apply threshold to envelope
                float currentGainReduction = parameters.threshold - Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample));
                // remove positive gain reduction
                currentGainReduction = jmin(0.0f, currentGainReduction);
                // set gr meter values
                outputGainReduction[channel] = jmin(currentGainReduction, outputGainReduction[channel]);
                // add autogain and apply ceiling
                currentGainReduction += parameters.ceiling - parameters.threshold;
                // convert decibels to gain
                currentGainReduction = std::pow(10.0f, 0.05f * currentGainReduction);
                // output compression multiplier
                compressionBuffer.setSample(channel, sample, currentGainReduction);
            }
        }
    }

    void applyLimiting(AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            FloatVectorOperations::multiply(buffer.getWritePointer(channel),
                compressionBuffer.getReadPointer(channel), bufferSize);
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    Parameters parameters;
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    AudioBuffer<float> compressionBuffer, envelopeBuffer;
};