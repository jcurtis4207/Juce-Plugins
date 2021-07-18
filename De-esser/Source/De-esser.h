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

class Deesser
{
public:
    Deesser()
    {
        reset();
    }

    ~Deesser() {}

    void reset()
    {
        compressionLevelLeft = 0.0f;
        compressionLevelRight = 0.0f;
    }

    void setDeesserParameters(juce::AudioProcessorValueTreeState& apvts)
    {
        crossoverFreq = apvts.getRawParameterValue("crossoverFreq")->load();
        threshold = apvts.getRawParameterValue("threshold")->load();
        float attackInput = apvts.getRawParameterValue("attack")->load();
        float releaseInput = apvts.getRawParameterValue("release")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        wide = apvts.getRawParameterValue("wide")->load();
        listen = apvts.getRawParameterValue("listen")->load();
        attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * (float)sampleRate));
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = static_cast<float>(newSampleRate);
        // initialize buffers
        lowBuffer.setSize(numChannels, maxBlockSize);
        highBuffer.setSize(numChannels, maxBlockSize);
        compressionBuffer.setSize(numChannels, maxBlockSize);
        envelopeBuffer.setSize(numChannels, maxBlockSize);
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = 2;
        // prepare processor chains
        lowChain.prepare(spec);
        highChain.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy the buffer into the filtering buffers
        juce::FloatVectorOperations::copy(lowBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(lowBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        juce::FloatVectorOperations::copy(highBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(highBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // setup filters
        lowChain.get<0>().setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        lowChain.get<0>().setCutoffFrequency(crossoverFreq);
        highChain.get<0>().setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        highChain.get<0>().setCutoffFrequency(crossoverFreq);
        // initialize dsp audio blocks
        juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
        juce::dsp::AudioBlock<float> highBlock(highBuffer);
        juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
        // process the filters
        lowChain.process(lowContext);
        highChain.process(highContext);
        // output only highpassed buffer
        if (listen)
        {
            juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), highBuffer.getReadPointer(0), bufferSize);
            juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), highBuffer.getReadPointer(1), bufferSize);
            // reset gain reduction
            outputGainReductionLeft = 0.0f;
            outputGainReductionRight = 0.0f;
        }
        else
        {
            // create compression envelope
            createEnvelope(highBuffer.getReadPointer(0), highBuffer.getReadPointer(1), envelopeBuffer.getWritePointer(0), envelopeBuffer.getWritePointer(1), bufferSize, stereo);
            // calculate gain reduction from envelope
            calculateGainReduction(envelopeBuffer.getReadPointer(0), envelopeBuffer.getReadPointer(1), compressionBuffer.getWritePointer(0), compressionBuffer.getWritePointer(1), bufferSize);
            // apply compression to high buffer
            juce::FloatVectorOperations::multiply(highBuffer.getWritePointer(0), compressionBuffer.getReadPointer(0), bufferSize);
            juce::FloatVectorOperations::multiply(highBuffer.getWritePointer(1), compressionBuffer.getReadPointer(1), bufferSize);
            // if wide apply compression to low buffer
            if (wide)
            {
                juce::FloatVectorOperations::multiply(lowBuffer.getWritePointer(0), compressionBuffer.getReadPointer(0), bufferSize);
                juce::FloatVectorOperations::multiply(lowBuffer.getWritePointer(1), compressionBuffer.getReadPointer(1), bufferSize);
            }
            // sum low and high buffers to output
            juce::FloatVectorOperations::add(outputBuffer.getChannelPointer(0), lowBuffer.getReadPointer(0), highBuffer.getReadPointer(0), bufferSize);
            juce::FloatVectorOperations::add(outputBuffer.getChannelPointer(1), lowBuffer.getReadPointer(1), highBuffer.getReadPointer(1), bufferSize);
        }
    }

    void createEnvelope(const float* inputLeft, const float* inputRight, float* outputLeft, float* outputRight, const int bufferSize, bool isStereo)
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (isStereo)
            {
                // find max of input samples
                const float maxSample = juce::jmax(std::abs(inputLeft[sample]), std::abs(inputRight[sample]));
                // apply attack
                if (compressionLevelLeft < maxSample)
                {
                    compressionLevelLeft = maxSample + attackTime * (compressionLevelLeft - maxSample);
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
                // apply attack to left
                if (compressionLevelLeft < sampleLeft)
                {
                    compressionLevelLeft = sampleLeft + attackTime * (compressionLevelLeft - sampleLeft);
                }
                // apply release to left
                else
                {
                    compressionLevelLeft = sampleLeft + releaseTime * (compressionLevelLeft - sampleLeft);
                }
                // apply attack to right
                if (compressionLevelRight < sampleRight)
                {
                    compressionLevelRight = sampleRight + attackTime * (compressionLevelRight - sampleRight);
                }
                // apply release to right
                else
                {
                    compressionLevelRight = sampleRight + releaseTime * (compressionLevelRight - sampleRight);
                }
                // write envelope
                outputLeft[sample] = compressionLevelLeft;
                outputRight[sample] = compressionLevelRight;
            }
        }
    }

    void calculateGainReduction(const float* envelopeLeft, const float* envelopeRight, float* outputLeft, float* outputRight, const int bufferSize)
    {
        outputGainReductionLeft = 0.0f;
        outputGainReductionRight = 0.0f;
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // apply threshold and ratio to envelope
            float gainReductionLeft = slope * (threshold - juce::Decibels::gainToDecibels(envelopeLeft[sample]));
            float gainReductionRight = slope * (threshold - juce::Decibels::gainToDecibels(envelopeRight[sample]));
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
            // convert decibels to gain values
            gainReductionLeft = std::pow(10.0f, 0.05f * gainReductionLeft);
            gainReductionRight = std::pow(10.0f, 0.05f * gainReductionRight);
            // output compression multiplier back to input
            outputLeft[sample] = gainReductionLeft;
            outputRight[sample] = gainReductionRight;
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
    float sampleRate{ 0.0f };
    float crossoverFreq{ 1000.0f };
    float threshold{ 0.0f };
    float attackTime{ 0.1f };
    float releaseTime{ 10.0f };
    float slope = 1.0f - (1.0f / 4.0f);
    bool stereo{ true };
    bool wide{ false };
    bool listen{ false };
    float compressionLevelLeft, compressionLevelRight;
    float outputGainReductionLeft, outputGainReductionRight;
    juce::AudioBuffer<float> lowBuffer, highBuffer;
    juce::AudioBuffer<float> compressionBuffer, envelopeBuffer;
    juce::dsp::ProcessorChain<juce::dsp::LinkwitzRileyFilter<float>> lowChain, highChain;
};