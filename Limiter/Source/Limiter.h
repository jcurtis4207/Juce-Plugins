/*
*   Limiter Module (adapted from Compression module)
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

class Limiter
{
public:
    Limiter()
    {
        reset();
    }

    ~Limiter() {}

    void reset()
    {
        compressionLevelLeft = 0.0f;
        compressionLevelRight = 0.0f;
    }

    void updateLimiterValues(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values
        threshold = apvts.getRawParameterValue("threshold")->load();
        ceiling = apvts.getRawParameterValue("ceiling")->load();
        float releaseInput = apvts.getRawParameterValue("release")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        // calculate release time
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
    }

    void prepare(const double inputSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        sideChainBuffer.setSize(numChannels, maxBlockSize);
        envelopeBuffer.setSize(numChannels, maxBlockSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy the buffer into the sidechain
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // create envelope
        createEnvelope(sideChainBuffer.getReadPointer(0), sideChainBuffer.getReadPointer(1), envelopeBuffer.getWritePointer(0),
            envelopeBuffer.getWritePointer(1), bufferSize, stereo);
        // calculate gain reduciton
        calculateGainReduction(sideChainBuffer.getWritePointer(0), sideChainBuffer.getWritePointer(1), envelopeBuffer.getReadPointer(0),
            envelopeBuffer.getReadPointer(1), bufferSize);
        // apply gain reduction to output
        juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(0), sideChainBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(1), sideChainBuffer.getReadPointer(1), bufferSize);
    }

    void createEnvelope(const float* inputLeft, const float* inputRight, float* outputLeft,
        float* outputRight, const int bufferSize, bool isStereo)
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (isStereo)
            {
                // find max of input samples
                const float maxSample = juce::jmax(std::abs(inputLeft[sample]), std::abs(inputRight[sample]));
                // apply instant attack
                if (compressionLevelLeft < maxSample) {
                    compressionLevelLeft = maxSample;
                }
                // apply release
                else 
                {
                    compressionLevelLeft = maxSample + releaseTime * (compressionLevelLeft - maxSample);
                }
                // write envelope
                outputLeft[sample] = compressionLevelLeft;
                outputRight[sample] = compressionLevelLeft;
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                // get absolute value of each sample
                const float sampleLeft = std::abs(inputLeft[sample]);
                const float sampleRight = std::abs(inputRight[sample]);
                // apply instant attack to left
                if (compressionLevelLeft < sampleLeft) {
                    compressionLevelLeft = sampleLeft;
                }
                // apply release to left
                else {
                    compressionLevelLeft = sampleLeft + releaseTime * (compressionLevelLeft - sampleLeft);
                }
                // apply instant attack to right
                if (compressionLevelRight < sampleRight) {
                    compressionLevelRight = sampleRight;
                }
                // apply release to right
                else {
                    compressionLevelRight = sampleRight + releaseTime * (compressionLevelRight - sampleRight);
                }
                // write envelope
                outputLeft[sample] = compressionLevelLeft;
                outputRight[sample] = compressionLevelRight;
            }
        }
    }

    void calculateGainReduction(float* inputLeft, float* inputRight, const float* envelopeLeft, const float* envelopeRight, const int bufferSize)
    {
        outputGainReductionLeft = 0.0f;
        outputGainReductionRight = 0.0f;
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // apply threshold to envelope
            float gainReductionLeft = threshold - juce::Decibels::gainToDecibels(envelopeLeft[sample]);
            float gainReductionRight = threshold - juce::Decibels::gainToDecibels(envelopeRight[sample]);
            // remove "negative" gain reduction
            gainReductionLeft = juce::jmin(0.0f, gainReductionLeft);
            gainReductionRight = juce::jmin(0.0f, gainReductionRight);
            // set gr meter values
            if (gainReductionLeft < outputGainReductionLeft)
            {
                outputGainReductionLeft = gainReductionLeft;
            }
            if (gainReductionRight < outputGainReductionRight)
            {
                outputGainReductionRight = gainReductionRight;
            }
            // convert decibels to gain values, add autogain, and add ceiling
            gainReductionLeft = std::pow(10.0f, 0.05f * (gainReductionLeft - threshold + ceiling));
            gainReductionRight = std::pow(10.0f, 0.05f * (gainReductionRight - threshold + ceiling));
            // output gain reduction back to input
            inputLeft[sample] = gainReductionLeft;
            inputRight[sample] = gainReductionRight;
        }
    }

    const float getGainReductionLeft()
    {
        return (outputGainReductionLeft * -1.0f);
    }

    const float getGainReductionRight()
    {
        return (outputGainReductionRight * -1.0f);
    }

private:
    double sampleRate{ 0.0f };
    float threshold{ 0.0f };
    float ceiling{ 0.0f };
    float releaseTime{ 1.0f };
    bool stereo{ true };
    float compressionLevelLeft, compressionLevelRight;
    float outputGainReductionLeft, outputGainReductionRight;
    juce::AudioBuffer<float> sideChainBuffer, envelopeBuffer;
};