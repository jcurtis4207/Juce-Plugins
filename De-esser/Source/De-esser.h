/*
*   De-esser Module
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
    float crossoverFreq, threshold, attackTime, releaseTime;
    bool stereo, wide, listen;
};

class Deesser
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts, bool isListen)
    {
        parameters.crossoverFreq = apvts.getRawParameterValue("crossoverFreq")->load();
        parameters.threshold = apvts.getRawParameterValue("threshold")->load();
        const float attackInput = apvts.getRawParameterValue("attack")->load();
        parameters.attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) *
            static_cast<float>(sampleRate)));
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        parameters.releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) *
            static_cast<float>(sampleRate)));
        parameters.stereo = apvts.getRawParameterValue("stereo")->load();
        parameters.wide = apvts.getRawParameterValue("wide")->load();
        parameters.listen = isListen;
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        lowBuffer.setSize(numOutputs, maxBlockSize);
        highBuffer.setSize(numOutputs, maxBlockSize);
        compressionBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numOutputs;
        lowChain.prepare(spec);
        highChain.prepare(spec);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        lowBuffer.makeCopyOf(inputBuffer, true);
        highBuffer.makeCopyOf(inputBuffer, true);
        applyFilters();
        createEnvelope();
        calculateGainReduction();
        applyCompression();
        writeOutput(inputBuffer);
    }

    std::array<float, numOutputs> getGainReduction()
    {
        return std::array<float, numOutputs>{
            outputGainReduction[0] * -1.0f, outputGainReduction[1] * -1.0f};
    }

private:
    void applyFilters()
    {
        lowChain.setType(dsp::LinkwitzRileyFilterType::lowpass);
        lowChain.setCutoffFrequency(parameters.crossoverFreq);
        highChain.setType(dsp::LinkwitzRileyFilterType::highpass);
        highChain.setCutoffFrequency(parameters.crossoverFreq);
        dsp::AudioBlock<float> lowBlock(lowBuffer);
        dsp::AudioBlock<float> highBlock(highBuffer);
        dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        dsp::ProcessContextReplacing<float> highContext(highBlock);
        lowChain.process(lowContext);
        highChain.process(highContext);
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
                const float maxSample = jmax(std::abs(highBuffer.getSample(0, sample)),
                    std::abs(highBuffer.getSample(1, sample)));
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
                    const float inputSample = std::abs(highBuffer.getSample(channel, sample));
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
                // apply threshold and ratio to envelope
                float currentGainReduction = slope * (parameters.threshold - Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample)));
                // remove positive gain reduction
                currentGainReduction = jmin(0.0f, currentGainReduction);
                // set gr meter value
                outputGainReduction[channel] = jmin(currentGainReduction, outputGainReduction[channel]);
                // convert decibels to gain
                currentGainReduction = std::pow(10.0f, 0.05f * currentGainReduction);
                // output compression multiplier to compression buffer
                compressionBuffer.setSample(channel, sample, currentGainReduction);
            }
        }
    }

    void applyCompression()
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            // apply compression to high buffer
            FloatVectorOperations::multiply(highBuffer.getWritePointer(channel),
                compressionBuffer.getReadPointer(channel), bufferSize);
            // if wide set, also apply compression to low buffer
            if (parameters.wide)
            {
                FloatVectorOperations::multiply(lowBuffer.getWritePointer(channel),
                    compressionBuffer.getReadPointer(channel), bufferSize);
            }
        }
    }

    void writeOutput(AudioBuffer<float>& buffer)
    {
        for (int channel = 0; channel < numOutputs; channel++)
        {
            // if listen set, output high only, else sum low and high
            buffer.copyFrom(channel, 0, highBuffer.getReadPointer(channel), bufferSize);
            if (!parameters.listen)
            {
                buffer.addFrom(channel, 0, lowBuffer.getReadPointer(channel), bufferSize);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float slope = 1.0f - (1.0f / 4.0f);
    Parameters parameters;
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    AudioBuffer<float> lowBuffer, highBuffer, compressionBuffer, envelopeBuffer;
    dsp::LinkwitzRileyFilter<float> lowChain, highChain;
};