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

class Clipper
{
public:
    Clipper() {}

    ~Clipper() {}

    void updateClipperValues(juce::AudioProcessorValueTreeState& apvts)
    {
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
    }

    void prepare(const double inputSampleRate, const int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        clipBuffer(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1));
    }

    // take in a buffer, clip based on threshold, apply gain based on ceiling, output to the same buffer
    void clipBuffer(float* bufferLeft, float* bufferRight)
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        float tempGainReduction[2] = { 0.0f, 0.0f };
        // get parameters in gain units
        const float thresholdHigh = juce::Decibels::decibelsToGain(threshold);
        const float thresholdLow = thresholdHigh * -1.0f;
        float autoGain = juce::Decibels::decibelsToGain(threshold * -1.0f);
        float ceilingGain = juce::Decibels::decibelsToGain(ceiling);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            float output[2] = { bufferLeft[sample], bufferRight[sample] };
            for (int channel = 0; channel < 2; channel++)
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
                // apply autogain and ceiling
                output[channel] *= autoGain;
                output[channel] *= ceilingGain;
                // calculate gain reduction
                outputGainReduction[channel] = (tempGainReduction[channel] > outputGainReduction[channel]) ? tempGainReduction[channel] : outputGainReduction[channel];
            }
            // write output to buffer
            bufferLeft[sample] = output[0];
            bufferRight[sample] = output[1];
        }
    }

    const float getGainReductionLeft()
    {
        return outputGainReduction[0];
    }
    const float getGainReductionRight()
    {
        return outputGainReduction[1];
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    float outputGainReduction[2]{ 0.0f, 0.0f };
};