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
    MultiBandComp(){}

    ~MultiBandComp(){}

    void setParameters(juce::AudioProcessorValueTreeState& apvts, bool* listenPtr)
    {
        // ensure crossovers don't overlap: B -> A -> C
        float tempA = apvts.getRawParameterValue("crossoverFreqA")->load();
        float tempB = apvts.getRawParameterValue("crossoverFreqB")->load();
        float tempC = apvts.getRawParameterValue("crossoverFreqC")->load();
        if (tempA < (tempB * 1.25f))
        {
            float sendValue = freqRange.convertTo0to1(tempB * 1.25f);
            apvts.getParameter("crossoverFreqA")->setValueNotifyingHost(sendValue);
            crossoverFreqA = tempB * 1.25f;
        }
        else
        {
            crossoverFreqA = tempA;
        }
        if (tempB > (tempA * 0.8f))
        {
            float sendValue = freqRange.convertTo0to1(tempA * 0.8f);
            apvts.getParameter("crossoverFreqB")->setValueNotifyingHost(sendValue);
            crossoverFreqB = tempA * 0.8f;
        }
        else
        {
            crossoverFreqB = tempB;
        }
        if (tempA > (tempC * 0.8f))
        {
            float sendValue = freqRange.convertTo0to1(tempC * 0.8f);
            apvts.getParameter("crossoverFreqA")->setValueNotifyingHost(sendValue);
            crossoverFreqA = tempC * 0.8f;
        }
        else
        {
            crossoverFreqA = tempA;
        }
        if (tempC < (tempA * 1.25f))
        {
            float sendValue = freqRange.convertTo0to1(tempA * 1.25f);
            apvts.getParameter("crossoverFreqC")->setValueNotifyingHost(sendValue);
            crossoverFreqC = tempA * 1.25f;
        }
        else
        {
            crossoverFreqC = tempC;
        }
        // get other parameter values
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
            listen[band] = listenPtr[band];
        }
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
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
        createEnvelopes();
        applyGainReduction();
        // use stage 1 buffers to build output
        stage1LowBuffer.clear();
        stage1HighBuffer.clear();
        for (int channel = 0; channel < 2; channel++)
        {
            for (int band = 0; band < 4; band++)
            {
                // build buffer of bands set to listen
                if (listen[band])
                {
                    juce::FloatVectorOperations::add(stage1LowBuffer.getWritePointer(channel), bandBuffers[band].getReadPointer(channel), bufferSize);
                }
                // build buffer of all bands
                juce::FloatVectorOperations::add(stage1HighBuffer.getWritePointer(channel), bandBuffers[band].getReadPointer(channel), bufferSize);
            }
            // set output based on listen status
            if (listen[0] || listen[1] || listen[2] || listen[3])
            {
                juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(channel), stage1LowBuffer.getReadPointer(channel), bufferSize);
            }
            else
            {
                juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(channel), stage1HighBuffer.getReadPointer(channel), bufferSize);
            }
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

    // create compression envelopes for each band
    void createEnvelopes()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for(int band = 0; band < 4; band++)
            {
                // stereo mode - max channel value used and output stored in both channels
                if (stereo)
                {
                    const float maxSample = juce::jmax(
                        std::abs(bandBuffers[band].getSample(0, sample)),std::abs(bandBuffers[band].getSample(1, sample)));
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
                    for (int channel = 0; channel < 2; channel++)
                    {
                        // get absolute value of sample
                        const float inputSample = std::abs(bandBuffers[band].getSample(channel, sample));
                        // apply attack
                        if (compressionLevel[band * 2 + channel] < inputSample)
                        {
                            compressionLevel[band * 2 + channel] = inputSample + attackTime[band] * (compressionLevel[band * 2 + channel] - inputSample);
                        }
                        // apply release
                        else
                        {
                            compressionLevel[band * 2 + channel] = inputSample + releaseTime[band] * (compressionLevel[band * 2 + channel] - inputSample);
                        }
                        // write envelope
                        envelopeBuffers[band].setSample(channel, sample, compressionLevel[band * 2 + channel]);
                    }
                }
            }
        }
    }

    // apply compression from envelope to band buffers
    void applyGainReduction()
    {
        for (int i = 0; i < 8; i++)
        {
            outputGainReduction[i] = 0.0f;
        }
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int band = 0; band < 4; band++)
            {
                for (int channel = 0; channel < 2; channel++)
                {
                    // apply threshold and ratio to envelope
                    float currentGainReduction = slope[band] * (threshold[band] - juce::Decibels::gainToDecibels(envelopeBuffers[band].getSample(channel, sample)));
                    // remove positive gain reduction
                    currentGainReduction = juce::jmin(0.0f, currentGainReduction);
                    // set gr meter values
                    if (currentGainReduction < outputGainReduction[band * 2 + channel])
                    {
                        outputGainReduction[band * 2 + channel] = currentGainReduction;
                    }
                    // convert decibels to gain and add makeup gain
                    currentGainReduction = std::pow(10.0f, 0.05f * (currentGainReduction + makeUpGain[band]));
                    // apply compression to buffer
                    bandBuffers[band].setSample(channel, sample, bandBuffers[band].getSample(channel, sample) * currentGainReduction);
                }
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
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    juce::NormalisableRange<float> freqRange{ 20.0f, 15000.0f, 1.0f, 0.25f };
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
    bool listen[4]{ false, false, false, false };
    float compressionLevel[8];
    float outputGainReduction[8];
    juce::AudioBuffer<float> bandBuffers[4];
    juce::AudioBuffer<float> envelopeBuffers[4];
};

/*  Signal Flow Diagram:
*                                                           |--- crossoverFreqC HPF -> band4 ---|
*          |--- crossoverFreqA HPF -> crossoverFreqB APF ---|                                   |
*          |                                                |--- crossoverFreqC LPF -> band3 ---|
* input ---|                                                                                    |--- output
*          |                                                |--- crossoverFreqB HPF -> band2 ---|
*          |--- crossoverFreqA LPF -> crossoverFreqC APF ---|                                   |
*                                                           |--- crossoverFreqB LPF -> band1 ---|
*/