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

#define numOutputs 2
#define numBands 4

using namespace juce;

struct Parameters {
    float crossoverFreqA, crossoverFreqB, crossoverFreqC;
    std::array<float, numBands> threshold;
    std::array<float, numBands> attackTime;
    std::array<float, numBands> releaseTime;
    std::array<float, numBands> slope;
    std::array<float, numBands> makeUpGain;
    bool stereo{ true };
    std::array<bool, numBands> listen{ false, false, false, false };
};

class MultiBandComp
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts, 
        const std::array<bool, numBands>& listenArr)
    {
        setCrossovers(apvts);
        anyListen = false;
        parameters.stereo = apvts.getRawParameterValue("stereo")->load();
        parameters.listen = listenArr;
        for (int band = 0; band < numBands; band++)
        {
            const auto bandNum = String(band + 1);
            parameters.threshold[band] = apvts.getRawParameterValue("threshold" + bandNum)->load();
            parameters.makeUpGain[band] = apvts.getRawParameterValue("makeUp" + bandNum)->load();
            const float attackInput = apvts.getRawParameterValue("attack" + bandNum)->load();
            parameters.attackTime[band] = std::exp(
                -1.0f / ((attackInput / 1000.0f) * static_cast<float>(sampleRate)));
            const float releaseInput = apvts.getRawParameterValue("release" + bandNum)->load();
            parameters.releaseTime[band] = std::exp(
                -1.0f / ((releaseInput / 1000.0f) * static_cast<float>(sampleRate)));
            const float ratio = apvts.getRawParameterValue("ratio" + bandNum)->load();
            parameters.slope[band] = 1.0f - (1.0f / ratio);
            if (listenArr[band])
            {
                anyListen = true;
            }
        }
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numOutputs;
        stage1LowBuffer.setSize(numOutputs, maxBlockSize);
        stage1HighBuffer.setSize(numOutputs, maxBlockSize);
        for (int band = 0; band < numBands; band++)
        {
            bandBuffers[band].setSize(numOutputs, maxBlockSize);
            envelopeBuffers[band].setSize(numOutputs, maxBlockSize);
            bandChains[band].prepare(spec);
        }
        stage1LowChain.prepare(spec);
        stage1HighChain.prepare(spec);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        // stage 1
        stage1LowBuffer.makeCopyOf(inputBuffer, true);
        stage1HighBuffer.makeCopyOf(inputBuffer, true);
        applyStage1Filters();
        // stage 2
        bandBuffers[0].makeCopyOf(stage1LowBuffer, true);
        bandBuffers[1].makeCopyOf(stage1LowBuffer, true);
        bandBuffers[2].makeCopyOf(stage1HighBuffer, true);
        bandBuffers[3].makeCopyOf(stage1HighBuffer, true);
        applyStage2Filters();
        // compression
        createEnvelopes();
        applyCompression();
        outputActiveBands(inputBuffer);
    }

    std::array<std::array<float, numOutputs>, numBands> getGainReduction()
    {
        std::array<std::array<float, numOutputs>, numBands> output;
        for (int band = 0; band < numBands; band++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                output[band][channel] = outputGainReduction[channel + band * 2] * -1.0f;
            }
        }
        return output;
    }

private:
    void setCrossovers(const AudioProcessorValueTreeState& apvts)
    {
        const float tempA = apvts.getRawParameterValue("crossoverFreqA")->load();
        const float tempB = apvts.getRawParameterValue("crossoverFreqB")->load();
        const float tempC = apvts.getRawParameterValue("crossoverFreqC")->load();
        // ensure crossovers don't overlap: B -> A -> C
        parameters.crossoverFreqA = jmax(tempA, tempB * 1.25f);
        parameters.crossoverFreqB = jmin(tempB, tempA * 0.8f);
        parameters.crossoverFreqA = jmin(parameters.crossoverFreqA, tempC * 0.8f);
        parameters.crossoverFreqC = jmax(tempC, tempA * 1.25f);
        // send new values to parameters
        apvts.getParameter("crossoverFreqA")->setValueNotifyingHost(
            freqRange.convertTo0to1(parameters.crossoverFreqA));
        apvts.getParameter("crossoverFreqB")->setValueNotifyingHost(
            freqRange.convertTo0to1(parameters.crossoverFreqB));
        apvts.getParameter("crossoverFreqC")->setValueNotifyingHost(
            freqRange.convertTo0to1(parameters.crossoverFreqC));
    }

    void applyStage1Filters()
    {
        stage1LowChain.get<0>().setType(dsp::LinkwitzRileyFilterType::lowpass);
        stage1LowChain.get<0>().setCutoffFrequency(parameters.crossoverFreqA);
        stage1LowChain.get<1>().setType(dsp::LinkwitzRileyFilterType::allpass);
        stage1LowChain.get<1>().setCutoffFrequency(parameters.crossoverFreqC);
        stage1HighChain.get<0>().setType(dsp::LinkwitzRileyFilterType::highpass);
        stage1HighChain.get<0>().setCutoffFrequency(parameters.crossoverFreqA);
        stage1HighChain.get<1>().setType(dsp::LinkwitzRileyFilterType::allpass);
        stage1HighChain.get<1>().setCutoffFrequency(parameters.crossoverFreqB);
        dsp::AudioBlock<float> lowBlock(stage1LowBuffer);
        dsp::AudioBlock<float> highBlock(stage1HighBuffer);
        dsp::ProcessContextReplacing<float> lowContext(lowBlock);
        dsp::ProcessContextReplacing<float> highContext(highBlock);
        stage1LowChain.process(lowContext);
        stage1HighChain.process(highContext);
    }

    void applyStage2Filters()
    {
        bandChains[0].setType(dsp::LinkwitzRileyFilterType::lowpass);
        bandChains[0].setCutoffFrequency(parameters.crossoverFreqB);
        bandChains[1].setType(dsp::LinkwitzRileyFilterType::highpass);
        bandChains[1].setCutoffFrequency(parameters.crossoverFreqB);
        bandChains[2].setType(dsp::LinkwitzRileyFilterType::lowpass);
        bandChains[2].setCutoffFrequency(parameters.crossoverFreqC);
        bandChains[3].setType(dsp::LinkwitzRileyFilterType::highpass);
        bandChains[3].setCutoffFrequency(parameters.crossoverFreqC);
        for (int band = 0; band < numBands; band++)
        {
            dsp::AudioBlock<float> block(bandBuffers[band]);
            dsp::ProcessContextReplacing<float> context(block);
            bandChains[band].process(context);
        }
    }

    void applyHisteresis(float& compLevel, float inputSample, int band)
    {
        float histeresis = (compLevel < inputSample) ? 
            parameters.attackTime[band] : parameters.releaseTime[band];
        compLevel = inputSample + histeresis * (compLevel - inputSample);
    }

    void createEnvelopes()
    {
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int band = 0; band < numBands; band++)
            {
                if (parameters.stereo)
                {
                    const float maxSample = jmax(std::abs(bandBuffers[band].getSample(0, sample)),
                        std::abs(bandBuffers[band].getSample(1, sample)));
                    applyHisteresis(compressionLevel[band], maxSample, band);
                    for (int channel = 0; channel < numOutputs; channel++)
                    {
                        envelopeBuffers[band].setSample(channel, sample, compressionLevel[band]);
                    }
                }
                else
                {
                    for (int channel = 0; channel < numOutputs; channel++)
                    {
                        const float inputSample = std::abs(
                            bandBuffers[band].getSample(channel, sample));
                        applyHisteresis(compressionLevel[channel + band * 2], inputSample, band);
                        envelopeBuffers[band].setSample(
                            channel, sample, compressionLevel[channel + band * 2]);
                    }
                }
            }
        }
    }

    void applyCompression()
    {
        for (int band = 0; band < numBands * numOutputs; band++)
        {
            outputGainReduction[band] = 0.0f;
        }
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int band = 0; band < numBands; band++)
            {
                for (int channel = 0; channel < numOutputs; channel++)
                {
                    // apply threshold and ratio to envelope
                    float currentGainReduction = parameters.slope[band] * 
                        (parameters.threshold[band] - Decibels::gainToDecibels(
                            envelopeBuffers[band].getSample(channel, sample)));
                    // remove positive gain reduction
                    currentGainReduction = jmin(0.0f, currentGainReduction);
                    // set gr meter values
                    outputGainReduction[channel + band * 2] = jmin(
                        currentGainReduction, outputGainReduction[channel + band * 2]);
                    // convert decibels to gain and add makeup gain
                    currentGainReduction = std::pow(10.0f, 
                        0.05f * (currentGainReduction + parameters.makeUpGain[band]));
                    // apply compression to buffer
                    bandBuffers[band].setSample(channel, sample, 
                        bandBuffers[band].getSample(channel, sample) * currentGainReduction);
                }
            }
        }
    }

    void outputActiveBands(AudioBuffer<float>& buffer)
    {
        buffer.clear();
        for (int channel = 0; channel < numOutputs; channel++)
        {
            for (int band = 0; band < numBands; band++)
            {
                if (anyListen)
                {
                    if (parameters.listen[band])
                    {
                        buffer.addFrom(channel, 0, 
                            bandBuffers[band].getReadPointer(channel), bufferSize);
                    }
                }
                else
                {
                    buffer.addFrom(channel, 0, 
                        bandBuffers[band].getReadPointer(channel), bufferSize);
                }
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    bool anyListen{ false };
    Parameters parameters;
    const NormalisableRange<float> freqRange{ 20.0f, 15000.0f, 1.0f, 0.25f };
    std::array<float, numBands * numOutputs> compressionLevel;
    std::array<float, numBands * numOutputs> outputGainReduction;
    AudioBuffer<float> stage1LowBuffer, stage1HighBuffer;
    std::array<AudioBuffer<float>, numBands> bandBuffers;
    std::array<AudioBuffer<float>, numBands> envelopeBuffers;
    dsp::ProcessorChain<dsp::LinkwitzRileyFilter<float>,
        dsp::LinkwitzRileyFilter<float>> stage1LowChain, stage1HighChain;
   dsp::LinkwitzRileyFilter<float> bandChains[4];
};

/*  Signal Flow Diagram:
*                                             |--- xoFreqC HPF -> band4 ---|
*          |--- xoFreqA HPF -> xoFreqB APF ---|                            |
*          |                                  |--- xoFreqC LPF -> band3 ---|
* input ---|                                                               |--- output
*          |                                  |--- xoFreqB HPF -> band2 ---|
*          |--- xoFreqA LPF -> xoFreqC APF ---|                            |
*                                             |--- xoFreqB LPF -> band1 ---|
*/