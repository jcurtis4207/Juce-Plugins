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
        clipBuffer(upBlock.getChannelPointer(0), upBlock.getChannelPointer(1), oversampledBufferSize);
        oversampler.processSamplesDown(context.getOutputBlock());
        // clip at 1x oversampling
        clipBuffer(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1), bufferSize);
        // apply autogain and ceiling
        applyGain(outputBuffer.getChannelPointer(0), outputBuffer.getChannelPointer(1), bufferSize);
    }

    // take in a buffer, clip based on threshold, output to the same buffer
    void clipBuffer(float* bufferLeft, float* bufferRight, int blockSize)
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        float tempGainReduction[2] = { 0.0f, 0.0f };
        // get parameters in gain units
        const float thresholdHigh = juce::Decibels::decibelsToGain(threshold);
        const float thresholdLow = thresholdHigh * -1.0f;
        for (int sample = 0; sample < blockSize; sample++)
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
                // calculate gain reduction
                outputGainReduction[channel] = (tempGainReduction[channel] > outputGainReduction[channel]) ? tempGainReduction[channel] : outputGainReduction[channel];
            }
            // write output to buffer
            bufferLeft[sample] = output[0];
            bufferRight[sample] = output[1];
        }
    }

    // take in a buffer, apply autogain and ceiling, output to same buffer
    void applyGain(float* bufferLeft, float* bufferRight, int blockSize)
    {
        float autoGain = juce::Decibels::decibelsToGain(threshold * -1.0f);
        float ceilingGain = juce::Decibels::decibelsToGain(ceiling);
        for (int sample = 0; sample < blockSize; sample++)
        {
            float output[2] = { bufferLeft[sample], bufferRight[sample] };
            for (int channel = 0; channel < 2; channel++)
            {
                output[channel] *= autoGain;
                output[channel] *= ceilingGain;
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
    const int getOversamplerLatency()
    {
        return (int)oversampler.getLatencyInSamples();
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    int oversampledBufferSize{ 0 };
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    float outputGainReduction[2]{ 0.0f, 0.0f };
    juce::dsp::Oversampling<float> oversampler{ 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};