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

    void setStereoMode(const bool isStereo)
    {
        stereo = isStereo;
    }

    // Computes the gain reduction separately for each side-chain signal. The values will be in decibels and will NOT contain the make-up gain.
    void computeGainReductionDualMono(const float* sideChainLeft, const float* sideChainRight, float* destinationLeft, float* destinationRight, const int numSamples)
    {
        maxGainReductionLeft = 0.0f;
        maxGainReductionRight = 0.0f;
        // for each sample in buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // convert samples to decibels
            const float levelInDecibelsLeft = 20.0f * std::log10(abs(sideChainLeft[sample]));
            const float levelInDecibelsRight = 20.0f * std::log10(abs(sideChainRight[sample]));
            // calculate overshoot and apply ratio
            const float overShootLeft = levelInDecibelsLeft - threshold;
            const float overShootRight = levelInDecibelsRight - threshold;
            const float gainReductionLeft = (overShootLeft <= 0.0f) ? 0.0f : slope * overShootLeft;
            const float gainReductionRight = (overShootRight <= 0.0f) ? 0.0f : slope * overShootRight;
            // apply ballistics to left
            const float diffLeft = gainReductionLeft - stateLeft;
            if (diffLeft < 0.0f)
            {
                // wanted gain reduction is below state -> attack phase
                stateLeft += attackTime * diffLeft;
            }
            else
            {
                // release phase
                stateLeft += releaseTime * diffLeft;
            }
            // apply ballistics to right
            const float diffRight = gainReductionRight - stateRight;
            if (diffRight < 0.0f)
            {
                // wanted gain reduction is below state -> attack phase
                stateRight += attackTime * diffRight;
            }
            else
            {
                // release phase
                stateRight += releaseTime * diffRight;
            }
            // write back gain reduction
            destinationLeft[sample] = stateLeft;
            destinationRight[sample] = stateRight;
            // set gain reduction values
            if (stateLeft < maxGainReductionLeft)
            {
                maxGainReductionLeft = stateLeft;
            }
            if (stateRight < maxGainReductionRight)
            {
                maxGainReductionRight = stateRight;
            }
        }
        // convert gain to linear values and add makeup gain
        for (int i = 0; i < numSamples; ++i)
        {
            destinationLeft[i] = std::pow(10.0f, 0.05f * (destinationLeft[i] + makeUpGain));
            destinationRight[i] = std::pow(10.0f, 0.05f * (destinationRight[i] + makeUpGain));
        }
    }

    // Computes the gain reduction for both side-chain signals. The values will be in decibels and will NOT contain the make-up gain.
    void computeGainReductionStereo(const float* sideChainLeft, const float* sideChainRight, float* destinationLeft, float* destinationRight, const int numSamples)
    {
        maxGainReductionLeft = 0.0f;
        maxGainReductionRight = 0.0f;
        // for each sample in buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // find max of input channels
            float maxSample = juce::jmax(sideChainLeft[sample], sideChainRight[sample]);

            // convert samples to decibels
            const float levelInDecibels = 20.0f * std::log10(abs(maxSample));
            // calculate overshoot and apply ratio
            const float overShoot = levelInDecibels - threshold;
            const float gainReduction = (overShoot <= 0.0f) ? 0.0f : slope * overShoot;
            // apply ballistics
            const float diff = gainReduction - stateLeft;
            if (diff < 0.0f)
            {
                // wanted gain reduction is below state -> attack phase
                stateLeft += attackTime * diff;
            }
            else
            {
                // release phase
                stateLeft += releaseTime * diff;
            }
            // write back gain reduction to both channels
            destinationLeft[sample] = stateLeft;
            destinationRight[sample] = stateLeft;
            // set gain reduction value
            if (stateLeft < maxGainReductionLeft)
            {
                maxGainReductionLeft = stateLeft;
                maxGainReductionRight = stateLeft;
            }
        }
        // convert gain to linear values and add makeup gain
        for (int i = 0; i < numSamples; ++i)
        {
            destinationLeft[i] = std::pow(10.0f, 0.05f * (destinationLeft[i] + makeUpGain));
            destinationRight[i] = std::pow(10.0f, 0.05f * (destinationRight[i] + makeUpGain));
        }
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        // initialize buffers
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        grBuffer.setSize(numChannels, maxBlockSize);
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
        // calculate gain reduction based on stereo mode
        if (stereo)
        {
            computeGainReductionStereo(sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), grBuffer.getWritePointer(0), grBuffer.getWritePointer(1), numSamples);
        }
        else
        {
            computeGainReductionDualMono(sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), grBuffer.getWritePointer(0), grBuffer.getWritePointer(1), numSamples);
        }
        // apply gain reduction to output channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            juce::FloatVectorOperations::multiply(outBlock.getChannelPointer(channel), grBuffer.getReadPointer(channel), numSamples);
        }
    }

    void reset()
    {
        stateLeft = 0.0f;
        stateRight = 0.0f;
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
    float threshold{ -10.0f };
    float attackTime{ 10.0f };
    float releaseTime{ 50.0f };
    float slope{ 0.3f };
    float makeUpGain{ 0.0f };
    float scFreq{ 20.0f };
    bool scBypass{ true };
    bool stereo{ true };
    // state variables
    float stateLeft, stateRight;
    // gain reduction variables
    std::atomic<float> maxGainReductionLeft{ 0 };
    std::atomic<float> maxGainReductionRight{ 0 };
    // create sidechain buffer and gain reduction buffers
    juce::AudioBuffer<float> sideChainBuffer, grBuffer;
    // create sidechain filters
    juce::IIRFilter leftFilter, rightFilter;
};