/*
*   Distortion Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

using namespace juce;

struct Parameters {
    float drive, volume, offset, mix, anger, 
        hpfFreq, lpfFreq, shape;
    int distortionType;
    bool shapeTilt;
};

class Distortion
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.drive = apvts.getRawParameterValue("drive")->load();
        parameters.volume = apvts.getRawParameterValue("volume")->load();
        parameters.offset = apvts.getRawParameterValue("offset")->load() * 0.005f;
        parameters.mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        parameters.anger = apvts.getRawParameterValue("anger")->load();
        parameters.distortionType = static_cast<int>(apvts.getRawParameterValue("type")->load());
        parameters.hpfFreq = apvts.getRawParameterValue("hpf")->load();
        parameters.lpfFreq = apvts.getRawParameterValue("lpf")->load();
        parameters.shape = apvts.getRawParameterValue("shape")->load();
        parameters.shapeTilt = apvts.getRawParameterValue("shapeTilt")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate * 4.0;
        bufferSize = maxBlockSize * 4;
        dryBuffer.setSize(numOutputs, bufferSize);
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        filterChain.prepare(spec);
        dcFilter.prepare(spec);
        *dcFilter.state = *dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(10.0f, sampleRate, 4)[0];
        oversampler.reset();
        oversampler.initProcessing(bufferSize);
    }

    void process(const dsp::ProcessContextReplacing<float>& context)
    {
        auto upsampleBlock = oversampler.processSamplesUp(context.getInputBlock());
        for (int channel = 0; channel < numOutputs; channel++)
        {
            dryBuffer.copyFrom(channel, 0, upsampleBlock.getChannelPointer(channel), bufferSize);
        }
        applyInputFilters(upsampleBlock);
        distortBuffer(upsampleBlock);
        applyDcFilter(upsampleBlock);
        applyMix(upsampleBlock, dryBuffer);
        oversampler.processSamplesDown(context.getOutputBlock());
    }

    int getOversamplerLatency()
    {
        return static_cast<int>(oversampler.getLatencyInSamples());
    }

private:
    void applyInputFilters(dsp::AudioBlock<float>& block)
    {
        *filterChain.get<FilterChainIndex::HPF>().state = *dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(parameters.hpfFreq, sampleRate, 2)[0];
        *filterChain.get<FilterChainIndex::LPF>().state = *dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(parameters.lpfFreq, sampleRate, 2)[0];
        *filterChain.get<FilterChainIndex::LowShelf>().state = *dsp::IIR::Coefficients<float>::
            makeLowShelf(sampleRate, 900.0f, 0.4f, Decibels::decibelsToGain(parameters.shape * -1.0f));
        *filterChain.get<FilterChainIndex::HighShelf>().state = *dsp::IIR::Coefficients<float>::
            makeHighShelf(sampleRate, 900.0f, 0.4f, Decibels::decibelsToGain(parameters.shape));
        // bypass high shelf if tilt disabled
        filterChain.setBypassed<FilterChainIndex::HighShelf>(!parameters.shapeTilt);
        dsp::ProcessContextReplacing<float> filterContext(block);
        filterChain.process(filterContext);
    }

    void applyDcFilter(dsp::AudioBlock<float>& block)
    {
        dsp::ProcessContextReplacing<float> filterContext(block);
        dcFilter.process(filterContext);
    }

    void distortBuffer(dsp::AudioBlock<float>& block)
    {
        const float outputGain = Decibels::decibelsToGain(parameters.volume);
        const float autoGain = Decibels::decibelsToGain(parameters.drive / -5.0f) * 
            (-0.7f * parameters.anger + 1.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                float wetSample = block.getSample(channel, sample);
                wetSample *= (parameters.drive / 10.0f) + 1.0f;         // apply drive
                wetSample += parameters.offset;                         // apply dc offset
                distortSample(wetSample, parameters.distortionType);    // apply distortion
                wetSample *= autoGain;                                  // apply autogain
                wetSample *= outputGain;                                // apply volume
                block.setSample(channel, sample, wetSample);
            }
        }
    }

    void distortSample(float& sample, int type)
    {
        float angerValue;
        switch (type)
        {
        case 0: // inverse absolute value
            angerValue = -0.9f * parameters.anger + 1.0f;
            sample = sample / (angerValue + abs(sample));
            break;
        case 1: // arctan
            angerValue = -2.5f * parameters.anger + 3.0f;
            sample = (2.0f / float_Pi) * atan((float_Pi / angerValue) * sample);
            break;
        case 2: // erf
            angerValue = -2.5f * parameters.anger + 3.0f;
            sample = erf(sample * sqrt(float_Pi) / angerValue);
            break;
        case 3: // inverse square root
            angerValue = 4.5f * parameters.anger + 0.5f;
            sample = sample / sqrt((1.0f / angerValue) + (sample * sample));
            break;
        }
    }

    void applyMix(dsp::AudioBlock<float>& wetBlock, const AudioBuffer<float>& dryBlock)
    {
        const float dryMix = std::pow(std::sin(0.5f * float_Pi * (1.0f - parameters.mix)), 2.0f);
        const float wetMix = std::pow(std::sin(0.5f * float_Pi * parameters.mix), 2.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float wetSample = wetBlock.getSample(channel, sample) * wetMix;
                const float drySample = dryBlock.getSample(channel, sample) * dryMix;
                wetBlock.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    Parameters parameters;
    AudioBuffer<float> dryBuffer;
    using StereoFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, 
        dsp::IIR::Coefficients<float>>;
    dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter> filterChain;
    enum FilterChainIndex{ HPF, LPF, LowShelf, HighShelf };
    StereoFilter dcFilter;
    dsp::Oversampling<float> oversampler{ 2, 2, 
        dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};