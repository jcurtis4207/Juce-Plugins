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

using namespace juce;

struct Parameters {
    float threshold, attackTime, releaseTime, slope, makeUpGain, scFreq, mix;
    bool scBypass, stereo;
};

class Compressor
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.threshold = apvts.getRawParameterValue("threshold")->load();
        const float attackInput = apvts.getRawParameterValue("attack")->load();
        parameters.attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * static_cast<float>(sampleRate)));
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        parameters.releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * static_cast<float>(sampleRate)));
        const float ratio = apvts.getRawParameterValue("ratio")->load();
        parameters.slope = 1.0f - (1.0f / ratio);
        parameters.makeUpGain = apvts.getRawParameterValue("makeUp")->load();
        parameters.scFreq = apvts.getRawParameterValue("scFreq")->load();
        parameters.scBypass = apvts.getRawParameterValue("scBypass")->load();
        parameters.stereo = apvts.getRawParameterValue("stereo")->load();
        const float mixInput = apvts.getRawParameterValue("mix")->load();
        parameters.mix = mixInput / 100.0f;
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        dryBuffer.setSize(numOutputs, maxBlockSize);
        wetBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        dryBuffer.makeCopyOf(inputBuffer, true);
        wetBuffer.makeCopyOf(inputBuffer, true);
        if (!parameters.scBypass)
        {
            applyFilters();
        }
        createEnvelope();
        applyCompression();
        mixToOutput(inputBuffer);
    }

    std::array<float, numOutputs> getGainReduction()
    {
        return std::array<float, numOutputs>{
            outputGainReduction[0] * -1.0f,
            outputGainReduction[1] * -1.0f
        };
    }

private:
    void applyFilters()
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            filters[channel].setCoefficients(IIRCoefficients::makeHighPass(sampleRate, parameters.scFreq));
            filters[channel].processSamples(wetBuffer.getWritePointer(channel), bufferSize);
        }
    }

    void applyHisteresis(float& compLevel, float inputSample)
    {
        float histeresis = (compLevel < inputSample) ? parameters.attackTime : parameters.releaseTime;
        compLevel = inputSample + histeresis * (compLevel - inputSample);
    }

    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            if (parameters.stereo)
            {
                const float maxSample = jmax(std::abs(wetBuffer.getSample(0, sample)),
                    std::abs(wetBuffer.getSample(1, sample)));
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
                    const float inputSample = std::abs(wetBuffer.getSample(channel, sample));
                    applyHisteresis(compressionLevel[channel], inputSample);
                    envelopeBuffer.setSample(channel, sample, compressionLevel[channel]);
                }
            }
        }
    }

    void applyCompression()
    {
        outputGainReduction = { 0.0f, 0.0f };
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // apply threshold and ratio to envelope
                float currentGainReduction = parameters.slope * (parameters.threshold - Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample)));
                // remove positive gain reduction
                currentGainReduction = jmin(0.0f, currentGainReduction);
                // set meter values
                outputGainReduction[channel] = jmin(currentGainReduction, outputGainReduction[channel]);
                // add makeup gain and convert decibels to gain
                currentGainReduction = std::pow(10.0f, 0.05f * (currentGainReduction + parameters.makeUpGain));
                // output compressed signal
                wetBuffer.setSample(channel, sample, 
                    dryBuffer.getSample(channel, sample) * currentGainReduction);
            }
        }
    }

    void mixToOutput(AudioBuffer<float>& buffer)
    {
        const float dryMix = std::pow(std::sin(0.5f * float_Pi * (1.0f - parameters.mix)), 2.0f);
        const float wetMix = std::pow(std::sin(0.5f * float_Pi * parameters.mix), 2.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float wetSample = wetBuffer.getSample(channel, sample) * wetMix;
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                buffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    Parameters parameters;
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    AudioBuffer<float> dryBuffer, wetBuffer, envelopeBuffer;
    std::array<IIRFilter, numOutputs> filters;
};