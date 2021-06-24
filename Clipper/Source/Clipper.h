/*
*   Clipper Module
*       by Jacob Curtis
*
*   Adapted from SimpleClipper by Daniel Rudrich
*   Licensed under the GPL Version 3
*   https://github.com/DanielRudrich/SimpleClipper
*   Copyright (c) 2019 Daniel Rudrich
*
*
*/

#pragma once
#include <JuceHeader.h>
#include <limits>
#include <atomic>

class Clipper
{
public:
    Clipper() {}

    ~Clipper() {}

    void updateClipperValues(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
    }

    // take in a buffer, clip based on threshold, apply gain based on ceiling, output to a buffer
    void clipBuffer(const float* inputLeft, const float* inputRight, float* outputLeft, float* outputRight, const int numSamples)
    {
        maxGainReductionLeft = 0.0f;
        maxGainReductionRight = 0.0f;
        float tempGainReductionLeft = 0.0f;
        float tempGainReductionRight = 0.0f;
        // for each sample in buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // get threshold in gain units
            const float thresholdHigh = juce::Decibels::decibelsToGain(threshold);
            const float thresholdLow = thresholdHigh * -1.0f;
            // clip left
            if (inputLeft[sample] > thresholdHigh)
            {
                outputLeft[sample] = thresholdHigh;
                tempGainReductionLeft = juce::Decibels::gainToDecibels(inputLeft[sample]) - threshold;
            }
            else if (inputLeft[sample] < thresholdLow)
            {
                outputLeft[sample] = thresholdLow;
                tempGainReductionLeft = juce::Decibels::gainToDecibels(inputLeft[sample]) - threshold;
            }
            else
            {
                outputLeft[sample] = inputLeft[sample];
            }
            // clip right
            if (inputRight[sample] > thresholdHigh)
            {
                outputRight[sample] = thresholdHigh;
                tempGainReductionRight = juce::Decibels::gainToDecibels(inputRight[sample]) - threshold;
            }
            else if (inputRight[sample] < thresholdLow)
            {
                outputRight[sample] = thresholdLow;
                tempGainReductionRight = juce::Decibels::gainToDecibels(inputRight[sample]) - threshold;
            }
            else
            {
                outputRight[sample] = inputRight[sample];
            }
            // apply autogain
            float autoGain = juce::Decibels::decibelsToGain(threshold * -1.0f);
            outputLeft[sample] *= autoGain;
            outputRight[sample] *= autoGain;
            // apply ceiling
            float ceilingGain = juce::Decibels::decibelsToGain(ceiling);
            outputLeft[sample] *= ceilingGain;
            outputRight[sample] *= ceilingGain;
            // calculate gain reduction
            maxGainReductionLeft = (tempGainReductionLeft > maxGainReductionLeft) ? tempGainReductionLeft : maxGainReductionLeft;
            maxGainReductionRight = (tempGainReductionRight > maxGainReductionRight) ? tempGainReductionRight : maxGainReductionRight;
        }
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        clippedBuffer.setSize(numChannels, maxBlockSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context, double inputSampleRate)
    {
        auto outBlock = context.getOutputBlock();
        const int numChannels = static_cast<int> (outBlock.getNumChannels());
        const int numSamples = static_cast<int> (outBlock.getNumSamples());
        // copy audio block into sidechain buffer
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(0), outBlock.getChannelPointer(0), numSamples);
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(1), outBlock.getChannelPointer(1), numSamples);
        // clip sidechain buffer and output to clipped buffer
        clipBuffer(sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), clippedBuffer.getWritePointer(0), clippedBuffer.getWritePointer(1), numSamples);
        // copy clipped buffer to output audio block
        juce::FloatVectorOperations::copy(outBlock.getChannelPointer(0), clippedBuffer.getReadPointer(0), numSamples);
        juce::FloatVectorOperations::copy(outBlock.getChannelPointer(1), clippedBuffer.getReadPointer(1), numSamples);
    }

    const float getGainReductionLeft()
    {
        return maxGainReductionLeft;
    }
    const float getGainReductionRight()
    {
        return maxGainReductionRight;
    }

private:
    double sampleRate{ 0.0f };
    // parameters
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    // gain reduction values
    std::atomic<float> maxGainReductionLeft{ 0.0f };
    std::atomic<float> maxGainReductionRight{ 0.0f };
    // create processing buffers
    juce::AudioBuffer<float> sideChainBuffer, clippedBuffer;;
};