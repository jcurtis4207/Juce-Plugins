/*
*   Clipper Module
*       by Jacob Curtis
*
*   Adapted from SimpleCompressor by Daniel Rudrich
*   Licensed under the GPL Version 3
*   https://github.com/DanielRudrich/SimpleCompressor
*
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

using namespace juce;

struct Parameters {
    float threshold, ceiling;
};

class Clipper
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.threshold = apvts.getRawParameterValue("threshold")->load();
        parameters.ceiling = apvts.getRawParameterValue("ceiling")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate * 4.0;
        bufferSize = maxBlockSize;
        oversampledBufferSize = maxBlockSize * 4;
        oversampler.reset();
        oversampler.initProcessing(oversampledBufferSize);
    }

    void process(const dsp::ProcessContextReplacing<float>& context)
    {
        auto& outputBuffer = context.getOutputBlock();
        auto upsampledBlock = oversampler.processSamplesUp(context.getInputBlock());
        // clip with 4x oversampling
        clipBuffer(upsampledBlock, oversampledBufferSize, oversampledGainReduction);
        oversampler.processSamplesDown(context.getOutputBlock());
        // clip with no oversampling
        clipBuffer(outputBuffer, bufferSize, normalGainReduction);
        applyGain(outputBuffer, bufferSize);
    }
    
    std::array<float, numOutputs> getGainReduction()
    {
        return std::array<float, numOutputs>{
            oversampledGainReduction[0] + normalGainReduction[0],
            oversampledGainReduction[1] + normalGainReduction[1]
        };
    }

    int getOversamplerLatency()
    {
        return (int)oversampler.getLatencyInSamples();
    }

private:
    void clipBuffer(dsp::AudioBlock<float>& block, int blockSize, 
        std::array<float, numOutputs>& outputGR)
    {
        outputGR = { 0.0f, 0.0f };
        std::array<float, numOutputs> tempGR = { 0.0f, 0.0f };
        const float thresholdHigh = Decibels::decibelsToGain(parameters.threshold);
        const float thresholdLow = thresholdHigh * -1.0f;
        for (int sample = 0; sample < blockSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float inputSample = block.getSample(channel, sample);
                const float outputSample = jmax(jmin(inputSample, thresholdHigh), thresholdLow);
                if (inputSample != outputSample)
                {
                    tempGR[channel] = Decibels::gainToDecibels(inputSample) - parameters.threshold;
                }
                outputGR[channel] = jmax(tempGR[channel], outputGR[channel]);
                block.setSample(channel, sample, outputSample);
            }
        }
    }

    void applyGain(dsp::AudioBlock<float>& block, int blockSize)
    {
        for (int sample = 0; sample < blockSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                float outputSample = block.getSample(channel, sample);
                outputSample *= Decibels::decibelsToGain(parameters.threshold * -1.0f); // autogain
                outputSample *= Decibels::decibelsToGain(parameters.ceiling);           // ceiling
                block.setSample(channel, sample, outputSample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    int oversampledBufferSize{ 0 };
    Parameters parameters;
    std::array<float, numOutputs> oversampledGainReduction{ 0.0f, 0.0f };
    std::array<float, numOutputs> normalGainReduction{ 0.0f, 0.0f };
    dsp::Oversampling<float> oversampler{ 2, 2, 
        dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};