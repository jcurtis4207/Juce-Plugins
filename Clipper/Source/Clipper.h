/*
*   Clipper Module
*       by Jacob Curtis
*
*   Adapted from SimpleClipper by Daniel Rudrich
*   Licensed under the GPL Version 3
*   https://github.com/DanielRudrich/SimpleClipper
*
*
*/

#pragma once
#include <JuceHeader.h>

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

    void prepare(const double inputSampleRate)
    {
        sampleRate = inputSampleRate;
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto outputBuffer = context.getOutputBlock();
        const int bufferSize = outputBuffer.getNumSamples();
        // clip output buffer
        clipBuffer(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1), bufferSize);
    }

    // take in a buffer, clip based on threshold, apply gain based on ceiling, output to the same buffer
    void clipBuffer(float* bufferLeft, float* bufferRight, const int bufferSize)
    {
        outputGainReductionLeft = 0.0f;
        outputGainReductionRight = 0.0f;
        float tempGainReductionLeft = 0.0f;
        float tempGainReductionRight = 0.0f;
        for (int sample = 0; sample < bufferSize; ++sample)
        {
            // get threshold in gain units
            const float thresholdHigh = juce::Decibels::decibelsToGain(threshold);
            const float thresholdLow = thresholdHigh * -1.0f;
            float outputLeft = bufferLeft[sample];
            float outputRight = bufferRight[sample];
            // clip left
            if (outputLeft > thresholdHigh)
            {
                tempGainReductionLeft = juce::Decibels::gainToDecibels(outputLeft) - threshold;
                outputLeft = thresholdHigh;
            }
            else if (outputLeft < thresholdLow)
            {
                tempGainReductionLeft = juce::Decibels::gainToDecibels(outputLeft) - threshold;
                outputLeft = thresholdLow;
            }
            // clip right
            if (outputRight > thresholdHigh)
            {
                tempGainReductionRight = juce::Decibels::gainToDecibels(outputRight) - threshold;
                outputRight = thresholdHigh;
            }
            else if (outputRight < thresholdLow)
            {
                tempGainReductionRight = juce::Decibels::gainToDecibels(outputRight) - threshold;
                outputRight = thresholdLow;
            }
            // apply autogain
            float autoGain = juce::Decibels::decibelsToGain(threshold * -1.0f);
            outputLeft *= autoGain;
            outputRight *= autoGain;
            // apply ceiling
            float ceilingGain = juce::Decibels::decibelsToGain(ceiling);
            outputLeft *= ceilingGain;
            outputRight *= ceilingGain;
            // write output to buffer
            bufferLeft[sample] = outputLeft;
            bufferRight[sample] = outputRight;
            // calculate gain reduction
            outputGainReductionLeft = (tempGainReductionLeft > outputGainReductionLeft) ? tempGainReductionLeft : outputGainReductionLeft;
            outputGainReductionRight = (tempGainReductionRight > outputGainReductionRight) ? tempGainReductionRight : outputGainReductionRight;
        }
    }

    const float getGainReductionLeft()
    {
        return outputGainReductionLeft;
    }
    const float getGainReductionRight()
    {
        return outputGainReductionRight;
    }

private:
    double sampleRate{ 0.0f };
    // parameters
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    // gain reduction values
    float outputGainReductionLeft, outputGainReductionRight;
};