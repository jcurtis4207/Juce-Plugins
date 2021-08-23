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

#define numOutputs 2

class Deesser
{
public:
    void setParameters(const juce::AudioProcessorValueTreeState& apvts, bool isListen)
    {
        crossoverFreq = apvts.getRawParameterValue("crossoverFreq")->load();
        threshold = apvts.getRawParameterValue("threshold")->load();
        const float attackInput = apvts.getRawParameterValue("attack")->load();
        attackTime = std::exp(-1.0f / ((attackInput / 1000.0f) * static_cast<float>(sampleRate)));
        const float releaseInput = apvts.getRawParameterValue("release")->load();
        releaseTime = std::exp(-1.0f / ((releaseInput / 1000.0f) * static_cast<float>(sampleRate)));
        stereo = apvts.getRawParameterValue("stereo")->load();
        wide = apvts.getRawParameterValue("wide")->load();
        listen = isListen;
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        lowBuffer.setSize(numOutputs, maxBlockSize);
        highBuffer.setSize(numOutputs, maxBlockSize);
        compressionBuffer.setSize(numOutputs, maxBlockSize);
        envelopeBuffer.setSize(numOutputs, maxBlockSize);
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numOutputs;
        lowChain.prepare(spec);
        highChain.prepare(spec);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // copy the buffer into the filtering buffers
        for (int channel = 0; channel < numOutputs; channel++)
        {
            lowBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
            highBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
        }
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
        // create compression envelope and calculate gain reduction
        createEnvelope();
        calculateGainReduction();
        for (int channel = 0; channel < numOutputs; channel++)
        {
            // apply compression to high buffer
            juce::FloatVectorOperations::multiply(highBuffer.getWritePointer(channel), 
                compressionBuffer.getReadPointer(channel), bufferSize);
            // if wide set, apply compression to low buffer
            if (wide)
            {
                juce::FloatVectorOperations::multiply(lowBuffer.getWritePointer(channel), 
                    compressionBuffer.getReadPointer(channel), bufferSize);
            }
            // if listen set, output high only, else sum low and high
            inputBuffer.copyFrom(channel, 0, highBuffer.getReadPointer(channel), bufferSize);
            if (!listen)
            {
                inputBuffer.addFrom(channel, 0, lowBuffer.getReadPointer(channel), bufferSize);
            }
        }
    }

    float getGainReductionLeft()
    {
        return (outputGainReduction[0] * -1.0f);
    }

    float getGainReductionRight()
    {
        return (outputGainReduction[1] * -1.0f);
    }

private:
    // use high buffer to compute compression envelope
    void createEnvelope()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // stereo mode - max channel value used and output stored in both channels
            if (stereo)
            {
                const float maxSample = juce::jmax(std::abs(highBuffer.getSample(0, sample)),
                    std::abs(highBuffer.getSample(1, sample)));
                // apply attack
                if (compressionLevel[0] < maxSample)
                {
                    compressionLevel[0] = maxSample + attackTime * (compressionLevel[0] - maxSample);
                }
                // apply release
                else
                {
                    compressionLevel[0] = maxSample + releaseTime * (compressionLevel[0] - maxSample);
                }
                // write envelope
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    envelopeBuffer.setSample(channel, sample, compressionLevel[0]);
                }
            }
            // dual mono mode - channels used individually and output stored separately
            else
            {
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    const float inputSample = std::abs(highBuffer.getSample(channel, sample));
                    // apply attack
                    if (compressionLevel[channel] < inputSample)
                    {
                        compressionLevel[channel] = inputSample + attackTime * (compressionLevel[channel] - inputSample);
                    }
                    // apply release
                    else
                    {
                        compressionLevel[channel] = inputSample + releaseTime * (compressionLevel[channel] - inputSample);
                    }
                    // write envelope
                    envelopeBuffer.setSample(channel, sample, compressionLevel[channel]);
                }
            }
        }
    }

    // use envelope buffer to calculate compression multiplier to compression buffer
    void calculateGainReduction()
    {
        outputGainReduction[0] = 0.0f;
        outputGainReduction[1] = 0.0f;
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                // apply threshold and ratio to envelope
                float currentGainReduction = slope * (threshold - juce::Decibels::gainToDecibels(
                    envelopeBuffer.getSample(channel, sample)));
                // remove positive gain reduction
                currentGainReduction = juce::jmin(0.0f, currentGainReduction);
                // set gr meter values
                if (currentGainReduction < outputGainReduction[channel])
                {
                    outputGainReduction[channel] = currentGainReduction;
                }
                currentGainReduction = std::pow(10.0f, 0.05f * currentGainReduction);
                // output compression multiplier to compression buffer
                compressionBuffer.setSample(channel, sample, currentGainReduction);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float crossoverFreq{ 1000.0f };
    float threshold{ 0.0f };
    float attackTime{ 0.1f };
    float releaseTime{ 10.0f };
    float slope = 1.0f - (1.0f / 4.0f);
    bool stereo{ true };
    bool wide{ false };
    bool listen{ false };
    std::array<float, numOutputs> compressionLevel{ 0.0f, 0.0f };
    std::array<float, numOutputs> outputGainReduction{ 0.0f, 0.0f };
    juce::AudioBuffer<float> lowBuffer, highBuffer, compressionBuffer, envelopeBuffer;
    juce::dsp::ProcessorChain<juce::dsp::LinkwitzRileyFilter<float>> lowChain, highChain;
};