/*
*   Multiband Compressor Module
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

class MultiBandComp
{
public:
    MultiBandComp()
    {
        reset();
    }

    ~MultiBandComp(){}

    void reset()
    {
        for (int i = 0; i < 8; i++)
        {
            compressionLevel[i] = 0.0f;
        }
    }

    void setParameters(juce::AudioProcessorValueTreeState& apvts)
    {
        crossoverFreqA = apvts.getRawParameterValue("crossoverFreqA")->load();
        crossoverFreqB = apvts.getRawParameterValue("crossoverFreqB")->load();
        crossoverFreqC = apvts.getRawParameterValue("crossoverFreqC")->load();
        stereo = apvts.getRawParameterValue("stereo")->load();
        for (int band = 0; band < 4; band++)
        {
            auto bandNum = juce::String(band + 1);
            threshold[band] = apvts.getRawParameterValue("threshold" + bandNum)->load();
            float attackInput = apvts.getRawParameterValue("attack" + bandNum)->load();
            float releaseInput = apvts.getRawParameterValue("release" + bandNum)->load();
            float ratio = apvts.getRawParameterValue("ratio" + bandNum)->load();
            makeUpGain[band] = apvts.getRawParameterValue("makeUp" + bandNum)->load();
            attackTime[band] = std::exp(-1.0f / ((attackInput / 1000.0f) * (float)sampleRate));
            releaseTime[band] = std::exp(-1.0f / ((releaseInput / 1000.0f) * (float)sampleRate));
            slope[band] = 1.0f - (1.0f / ratio);
        }
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        // initialize buffers
        stage1LowBuffer.setSize(numChannels, maxBlockSize);
        stage1HighBuffer.setSize(numChannels, maxBlockSize);
        for (int band = 0; band < 4; band++)
        {
            bandBuffers[band].setSize(numChannels, maxBlockSize);
            envelopeBuffers[band].setSize(numChannels, maxBlockSize);
        }
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = 2;
        // prepare processor chains
        stage1LowChain.prepare(spec);
        stage1HighChain.prepare(spec);
        for (int band = 0; band < 4; band++)
        {
            bandChains[band].prepare(spec);
        }
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy input into stage 1
        for (int channel = 0; channel < 2; channel++)
        {
            juce::FloatVectorOperations::copy(stage1LowBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
            juce::FloatVectorOperations::copy(stage1HighBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
        }
        // apply stage 1 filters
        applyStage1Filters();
        for (int channel = 0; channel < 2; channel++)
        {
            // stage1LowBuffer -> band 1 & 2 buffers
            juce::FloatVectorOperations::copy(bandBuffers[0].getWritePointer(channel), stage1LowBuffer.getReadPointer(channel), bufferSize);
            juce::FloatVectorOperations::copy(bandBuffers[1].getWritePointer(channel), stage1LowBuffer.getReadPointer(channel), bufferSize);
            // stage1HighBuffer -> band 3 & 4 buffers
            juce::FloatVectorOperations::copy(bandBuffers[2].getWritePointer(channel), stage1HighBuffer.getReadPointer(channel), bufferSize);
            juce::FloatVectorOperations::copy(bandBuffers[3].getWritePointer(channel), stage1HighBuffer.getReadPointer(channel), bufferSize);
        }
        // apply stage 2 filters
        applyStage2Filters();
        // apply compression to filtered buffers
        createEnvelopes(bufferSize);
        applyGainReduction(bufferSize);
        for (int channel = 0; channel < 2; channel++)
        {
            // sum bands 1 & 2 to stage1Low
            juce::FloatVectorOperations::add(stage1LowBuffer.getWritePointer(channel), bandBuffers[0].getReadPointer(channel), bandBuffers[1].getReadPointer(channel), bufferSize);
            // sum bands 3 & 4 to stage1High
            juce::FloatVectorOperations::add(stage1HighBuffer.getWritePointer(channel), bandBuffers[2].getReadPointer(channel), bandBuffers[3].getReadPointer(channel), bufferSize);
            // sum stage 1 to output
            juce::FloatVectorOperations::add(outputBuffer.getChannelPointer(channel), stage1LowBuffer.getReadPointer(channel), stage1HighBuffer.getReadPointer(channel), bufferSize);
        }
    }

    void applyStage1Filters()
    {
        // setup filters
        stage1LowChain.get<0>().setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        stage1LowChain.get<0>().setCutoffFrequency(crossoverFreqA);
        stage1LowChain.get<1>().setType(juce::dsp::LinkwitzRileyFilterType::allpass);
        stage1LowChain.get<1>().setCutoffFrequency(crossoverFreqC);
        stage1HighChain.get<0>().setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        stage1HighChain.get<0>().setCutoffFrequency(crossoverFreqA);
        stage1HighChain.get<1>().setType(juce::dsp::LinkwitzRileyFilterType::allpass);
        stage1HighChain.get<1>().setCutoffFrequency(crossoverFreqB);
        // initialize dsp audio blocks
        juce::dsp::AudioBlock<float> lowBlock(stage1LowBuffer);
        juce::dsp::AudioBlock<float> highBlock(stage1HighBuffer);
        juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        juce::dsp::ProcessContextReplacing<float> highContext(highBlock);
        // process the filters
        stage1LowChain.process(lowContext);
        stage1HighChain.process(highContext);
    }

    void applyStage2Filters()
    {
        // setup filters
        bandChains[0].get<0>().setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        bandChains[0].get<0>().setCutoffFrequency(crossoverFreqB);
        bandChains[1].get<0>().setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        bandChains[1].get<0>().setCutoffFrequency(crossoverFreqB);
        bandChains[2].get<0>().setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        bandChains[2].get<0>().setCutoffFrequency(crossoverFreqC);
        bandChains[3].get<0>().setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        bandChains[3].get<0>().setCutoffFrequency(crossoverFreqC);
        // initialize dsp audio blocks
        juce::dsp::AudioBlock<float> block0(bandBuffers[0]);
        juce::dsp::AudioBlock<float> block1(bandBuffers[1]);
        juce::dsp::AudioBlock<float> block2(bandBuffers[2]);
        juce::dsp::AudioBlock<float> block3(bandBuffers[3]);
        juce::dsp::ProcessContextReplacing<float> context0(block0);
        juce::dsp::ProcessContextReplacing<float> context1(block1);
        juce::dsp::ProcessContextReplacing<float> context2(block2);
        juce::dsp::ProcessContextReplacing<float> context3(block3);
        // process filter chains
        bandChains[0].process(context0);
        bandChains[1].process(context1);
        bandChains[2].process(context2);
        bandChains[3].process(context3);
    }

    void createEnvelopes(const int bufferSize)
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for(int band = 0; band < 4; band++)
            {
                // stereo mode - max channel value used and output stored in both channels
                if (stereo)
                {
                    // find max of input samples
                    const float sampleLeft = bandBuffers[band].getSample(0, sample);
                    const float sampleRight = bandBuffers[band].getSample(1, sample);
                    const float maxSample = juce::jmax(std::abs(sampleLeft), std::abs(sampleRight));
                    // apply attack
                    if (compressionLevel[band] < maxSample)
                    {
                        compressionLevel[band] = maxSample + attackTime[band] * (compressionLevel[band] - maxSample);
                    }
                    // apply release
                    else
                    {
                        compressionLevel[band] = maxSample + releaseTime[band] * (compressionLevel[band] - maxSample);
                    }
                    // write envelope
                    envelopeBuffers[band].setSample(0, sample, compressionLevel[band]);
                    envelopeBuffers[band].setSample(1, sample, compressionLevel[band]);
                }
                // dual mono mode - channels used individually and output stored separately
                else
                {
                    // get absolute value of each sample
                    const float sampleLeft = std::abs(bandBuffers[band].getSample(0, sample));
                    const float sampleRight = std::abs(bandBuffers[band].getSample(1, sample));
                    // apply attack to left
                    if (compressionLevel[band] < sampleLeft)
                    {
                        compressionLevel[band] = sampleLeft + attackTime[band] * (compressionLevel[band] - sampleLeft);
                    }
                    // apply release to left
                    else
                    {
                        compressionLevel[band] = sampleLeft + releaseTime[band] * (compressionLevel[band] - sampleLeft);
                    }
                    // apply attack to right
                    if (compressionLevel[band + 4] < sampleRight)
                    {
                        compressionLevel[band + 4] = sampleRight + attackTime[band] * (compressionLevel[band + 4] - sampleRight);
                    }
                    // apply release to right
                    else
                    {
                        compressionLevel[band + 4] = sampleRight + releaseTime[band] * (compressionLevel[band + 4] - sampleRight);
                    }
                    // write envelope
                    envelopeBuffers[band].setSample(0, sample, compressionLevel[band]);
                    envelopeBuffers[band].setSample(1, sample, compressionLevel[band + 4]);
                }
            }
        }
    }

    void applyGainReduction(const int bufferSize)
    {
        for (int i = 0; i < 8; i++)
        {
            outputGainReduction[i] = 0.0f;
        }
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int band = 0; band < 4; band++)
            {
                // apply threshold and ratio to envelope
                float gainReductionLeft = slope[band] * (threshold[band] - juce::Decibels::gainToDecibels(envelopeBuffers[band].getSample(0, sample)));
                float gainReductionRight = slope[band] * (threshold[band] - juce::Decibels::gainToDecibels(envelopeBuffers[band].getSample(1, sample)));
                // remove "negative" gain reduction
                gainReductionLeft = juce::jmin(0.0f, gainReductionLeft);
                gainReductionRight = juce::jmin(0.0f, gainReductionRight);
                // set gr meter values
                if (gainReductionLeft < outputGainReduction[band])
                {
                    outputGainReduction[band] = gainReductionLeft;
                }
                if (gainReductionRight < outputGainReduction[band + (int16_t)4])
                {
                    outputGainReduction[band + (int16_t)4] = gainReductionRight;
                }
                // add makeup gain and convert decibels to gain values
                gainReductionLeft = std::pow(10.0f, 0.05f * (gainReductionLeft + makeUpGain[band]));
                gainReductionRight = std::pow(10.0f, 0.05f * (gainReductionRight + makeUpGain[band]));
                // apply compression to buffer
                float dryLeft = bandBuffers[band].getSample(0, sample);
                float dryRight = bandBuffers[band].getSample(1, sample);
                bandBuffers[band].setSample(0, sample, dryLeft * gainReductionLeft);
                bandBuffers[band].setSample(1, sample, dryRight * gainReductionRight);
            }
        }
    }

    float* getGainReduction()
    {
        // invert gain reduction values and return pointer to array
        for (int i = 0; i < 8; i++)
        {
            outputGainReduction[i] *= -1.0f;
        }
        return outputGainReduction;
    }

private:
    double sampleRate;
    float crossoverFreqA, crossoverFreqB, crossoverFreqC;
    juce::AudioBuffer<float> stage1LowBuffer, stage1HighBuffer;
    juce::dsp::ProcessorChain<juce::dsp::LinkwitzRileyFilter<float>, juce::dsp::LinkwitzRileyFilter<float>> stage1LowChain, stage1HighChain;
    juce::dsp::ProcessorChain<juce::dsp::LinkwitzRileyFilter<float>> bandChains[4];
    float threshold[4];
    float attackTime[4];
    float releaseTime[4];
    float slope[4];
    float makeUpGain[4];
    bool stereo{ true };
    float compressionLevel[8];
    float outputGainReduction[8];
    juce::AudioBuffer<float> bandBuffers[4];
    juce::AudioBuffer<float> envelopeBuffers[4];
};

/*  Signal Flow Diagram:
*                                                           |--- crossoverFreqC HPF -> band4 ---|
*          |--- crossoverFreqA HPF -> crossoverFreqB APF ---|                                   |--- stage1High ---|
*          |                                                |--- crossoverFreqC LPF -> band3 ---|                  |
* input ---|                                                                                                       |--- output
*          |                                                |--- crossoverFreqB HPF -> band2 ---|                  |
*          |--- crossoverFreqA LPF -> crossoverFreqC APF ---|                                   |--- stage1Low ----|
*                                                           |--- crossoverFreqB LPF -> band1 ---|
*/