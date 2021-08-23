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

class Clipper
{
public:
    void setParameters(const juce::AudioProcessorValueTreeState& apvts)
    {
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate * 4.0;
        bufferSize = maxBlockSize;
        oversampledBufferSize = maxBlockSize * 4;
        oversampler.reset();
        oversampler.initProcessing(oversampledBufferSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        // clip at 4x oversampling
        auto upBlock = oversampler.processSamplesUp(context.getInputBlock());
        clipBuffer(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1), 
            oversampledBufferSize, oversampledGainReduction);
        oversampler.processSamplesDown(context.getOutputBlock());
        // clip at 1x oversampling
        clipBuffer(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1), 
            bufferSize, normalGainReduction);
        // apply autogain and ceiling
        applyGain(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1), bufferSize);
    }
    
    // get combined gain reduction of both clipping stages
    float getGainReductionLeft()
    {
        return oversampledGainReduction[0] + normalGainReduction[0];
    }
    float getGainReductionRight()
    {
        return oversampledGainReduction[1] + normalGainReduction[1];
    }
    int getOversamplerLatency()
    {
        return (int)oversampler.getLatencyInSamples();
    }

private:
    // take in a buffer, clip based on threshold, output to the same buffer
    void clipBuffer(float* bufferLeft, float* bufferRight, int blockSize,
        std::array<float, numOutputs>& stageGainReduction)
    {
        stageGainReduction = { 0.0f, 0.0f };
        std::array<float, numOutputs> tempGainReduction = { 0.0f, 0.0f };
        const float thresholdHigh = juce::Decibels::decibelsToGain(threshold);
        const float thresholdLow = thresholdHigh * -1.0f;
        for (int sample = 0; sample < blockSize; sample++)
        {
            std::array<float, numOutputs> output = { bufferLeft[sample], bufferRight[sample] };
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // clip positive values
                if (output[channel] > thresholdHigh)
                {
                    tempGainReduction[channel] = juce::Decibels::gainToDecibels(output[channel]) - threshold;
                    output[channel] = thresholdHigh;
                }
                // clip negative values
                else if (output[channel] < thresholdLow)
                {
                    tempGainReduction[channel] = juce::Decibels::gainToDecibels(output[channel]) - threshold;
                    output[channel] = thresholdLow;
                }
                // calculate gain reduction
                stageGainReduction[channel] = (tempGainReduction[channel] > stageGainReduction[channel]) ?
                    tempGainReduction[channel] : stageGainReduction[channel];
            }
            // write output to buffer
            bufferLeft[sample] = output[0];
            bufferRight[sample] = output[1];
        }
    }

    // take in a buffer, apply autogain and ceiling, output to same buffer
    void applyGain(float* bufferLeft, float* bufferRight, int blockSize)
    {
        for (int sample = 0; sample < blockSize; sample++)
        {
            std::array<float, numOutputs> output = { bufferLeft[sample], bufferRight[sample] };
            for (int channel = 0; channel < numOutputs; channel++)
            {
                output[channel] *= juce::Decibels::decibelsToGain(threshold * -1.0f);
                output[channel] *= juce::Decibels::decibelsToGain(ceiling);
            }
            // write output to buffer
            bufferLeft[sample] = output[0];
            bufferRight[sample] = output[1];
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    int oversampledBufferSize{ 0 };
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    std::array<float, numOutputs> oversampledGainReduction{ 0.0f, 0.0f };
    std::array<float, numOutputs> normalGainReduction{ 0.0f, 0.0f };
    juce::dsp::Oversampling<float> oversampler{ 2, 2, 
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};