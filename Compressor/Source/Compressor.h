/*
*   Compressor Module
*
*
*   Adapted from SimpleCompressor by Daniel Rudrich
*   Licensed under the GPL Version 3
*   https://github.com/DanielRudrich/SimpleCompressor
*   Copyright (c) 2019 Daniel Rudrich
*
*
*/

#pragma once
#include <JuceHeader.h>
#include <limits>
#include <atomic>

class Compressor
{
public:
    Compressor()
    {
        reset();
    }

    ~Compressor()
    {
    }

    void setAttackTime(const float attackTimeInMilliseconds)
    {
        attackTime = 1.0f - std::exp(-1.0f / (static_cast<float> (sampleRate) * (attackTimeInMilliseconds / 1000)));
    }

    void setReleaseTime(const float releaseTimeInMilliseconds)
    {
        releaseTime = 1.0f - std::exp(-1.0f / (static_cast<float> (sampleRate) * (releaseTimeInMilliseconds / 1000)));
    }

    void setThreshold(const float thresholdInDecibels)
    {
        threshold = thresholdInDecibels;
    }

    void setMakeUpGain(const float makeUpGainInDecibels)
    {
        makeUpGain = makeUpGainInDecibels;
    }

    void setRatio(const float ratio)
    {
        slope = 1.0f / ratio - 1.0f;
    }

    void setFilterFrequency(const float scFilterFrequency)
    {
        scFreq = scFilterFrequency;
    }

    void setFilterBypass(const bool scFilterBypass)
    {
        scBypass = scFilterBypass;
    }

    // Computes the gain reduction for a given side-chain signal. The values will be in decibels and will NOT contain the make-up gain.
    void computeGainFromSidechainSignal(const float* sideChainSignal, float* destination, const int numSamples)
    {
        maxInputLevel = -std::numeric_limits<float>::infinity();
        maxGainReduction = 0.0f;
        // for each sample in buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // convert sample to decibels
            const float levelInDecibels = 20.0f * std::log10(abs(sideChainSignal[sample]));
            // set new max level
            if (levelInDecibels > maxInputLevel)
            {
                maxInputLevel = levelInDecibels;
            }
            // calculate overshoot and apply ratio
            const float overShoot = levelInDecibels - threshold;
            const float gainReduction = (overShoot <= 0.0f) ? 0.0f : slope * overShoot;
            // apply ballistics
            const float diff = gainReduction - state;
            if (diff < 0.0f)
            {
                // wanted gain reduction is below state -> attack phase
                state += attackTime * diff;
            }
            else
            {
                // release phase
                state += releaseTime * diff;
            }
            // write back gain reduction
            destination[sample] = state;
            // set gain reduction value
            if (state < maxGainReduction)
            {
                maxGainReduction = state;
            }
        }
        // convert gain to linear values and add makeup gain
        for (int i = 0; i < numSamples; ++i)
        {
            destination[i] = std::pow(10.0f, 0.05f * (destination[i] + makeUpGain));
        }
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        // initialize sidechain buffer
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        // initialize sidechain filters
        auto hpfCoefficients = juce::IIRCoefficients::makeHighPass(newSampleRate, scFreq);
        leftFilter.setCoefficients(hpfCoefficients);
        rightFilter.setCoefficients(hpfCoefficients);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context, double inputSampleRate)
    {
        auto outBlock = context.getOutputBlock();
        const int numChannels = static_cast<int> (outBlock.getNumChannels());
        const int numSamples = static_cast<int> (outBlock.getNumSamples());

        /* STEP 1: get max buffer values and store in sidechain */
        // copy the absolute values from the input buffer to the sideChainBuffer
        juce::FloatVectorOperations::abs(sideChainBuffer.getWritePointer(0), outBlock.getChannelPointer(0), numSamples);
        juce::FloatVectorOperations::abs(sideChainBuffer.getWritePointer(1), outBlock.getChannelPointer(1), numSamples);

        // apply sidechain filter if not bypassed
        if (!scBypass)
        {
            auto hpfCoefficients = juce::IIRCoefficients::makeHighPass(inputSampleRate, scFreq);
            leftFilter.setCoefficients(hpfCoefficients);
            rightFilter.setCoefficients(hpfCoefficients);
            float* dataLeft = sideChainBuffer.getWritePointer(0);
            float* dataRight = sideChainBuffer.getWritePointer(1);
            leftFilter.processSamples(dataLeft, numSamples);
            rightFilter.processSamples(dataRight, numSamples);
        }

        // write max of both channels to channel 1
        juce::FloatVectorOperations::max(sideChainBuffer.getWritePointer(0), sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), numSamples);
        // sideChainBuffer[0] = max buffer absolute values

        /* STEP 2: calculate gain reduction */
        computeGainFromSidechainSignal(sideChainBuffer.getReadPointer(0), sideChainBuffer.getWritePointer(1), numSamples);
        // sideChainBuffer[1] = gain-reduction

        /* STEP 3: apply gain-reduction to all channels */
        for (int channel = 0; channel < numChannels; ++channel)
        {
            juce::FloatVectorOperations::multiply(outBlock.getChannelPointer(channel), sideChainBuffer.getReadPointer(1), numSamples);
        }
    }

    void reset()
    {
        state = 0.0f;
    }

    const float getMaxGainReductionInDecibels()
    {
        return maxGainReduction;
    }

private:
    double sampleRate{ 0.0f };
    // parameters
    float threshold{ -10.0f };
    float attackTime{ 10.0f };
    float releaseTime{ 50.0f };
    float slope{ 0.3f };
    float makeUpGain{ 0.0f };
    float scFreq{ 20.0f };
    bool scBypass{ true };
    // state variable
    float state;
    // atomic variables
    std::atomic<float> maxInputLevel{ -std::numeric_limits<float>::infinity() };
    std::atomic<float> maxGainReduction{ 0 };
    // create sidechain buffer
    juce::AudioBuffer<float> sideChainBuffer;
    // create sidechain filters
    juce::IIRFilter leftFilter, rightFilter;
};