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

class Distortion
{
public:
    Distortion(){}
    ~Distortion(){}

    void setParameters(const juce::AudioProcessorValueTreeState& apvts)
    {
        drive = apvts.getRawParameterValue("drive")->load();
        volume = apvts.getRawParameterValue("volume")->load();
        offset = apvts.getRawParameterValue("offset")->load() * 0.005f;
        mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        anger = apvts.getRawParameterValue("anger")->load();
        distortionType = static_cast<int>(apvts.getRawParameterValue("type")->load());
        hpfFreq = apvts.getRawParameterValue("hpf")->load();
        lpfFreq = apvts.getRawParameterValue("lpf")->load();
        shape = apvts.getRawParameterValue("shape")->load();
        shapeTilt = apvts.getRawParameterValue("shapeTilt")->load();
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate * 4.0;
        bufferSize = maxBlockSize * 4;
        dryBuffer.setSize(numOutputs, bufferSize);
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        filterChain.prepare(spec);
        dcFilter.prepare(spec);
        *dcFilter.state = *juce::dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(10.0f, sampleRate, 4)[0];
        oversampler.reset();
        oversampler.initProcessing(bufferSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto upBlock = oversampler.processSamplesUp(context.getInputBlock());
        for (int channel = 0; channel < numOutputs; channel++)
        {
            juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(channel), 
                upBlock.getChannelPointer(channel), bufferSize);
        }
        // apply filters, distortion, and mix
        applyInputFilters(upBlock);
        distortBuffer(upBlock);
        applyDcFilter(upBlock);
        applyMix(upBlock, dryBuffer);
        oversampler.processSamplesDown(context.getOutputBlock());
    }

    void applyInputFilters(juce::dsp::AudioBlock<float>& buffer)
    {
        // apply coefficients to filters
        *filterChain.get<FilterChainIndex::HPF>().state = *juce::dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2)[0];
        *filterChain.get<FilterChainIndex::LPF>().state = *juce::dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2)[0];
        *filterChain.get<FilterChainIndex::LowShelf>().state = *juce::dsp::IIR::Coefficients<float>::
            makeLowShelf(sampleRate, 900.0f, 0.4f, juce::Decibels::decibelsToGain(shape * -1.0f));
        *filterChain.get<FilterChainIndex::HighShelf>().state = *juce::dsp::IIR::Coefficients<float>::
            makeHighShelf(sampleRate, 900.0f, 0.4f, juce::Decibels::decibelsToGain(shape));
        // bypass high shelf if tilt disabled
        filterChain.setBypassed<FilterChainIndex::HighShelf>(!shapeTilt);
        // apply processing
        juce::dsp::ProcessContextReplacing<float> filterContext(buffer);
        filterChain.process(filterContext);
    }

    void applyDcFilter(juce::dsp::AudioBlock<float>& buffer)
    {
        juce::dsp::ProcessContextReplacing<float> filterContext(buffer);
        dcFilter.process(filterContext);
    }

    void distortBuffer(juce::dsp::AudioBlock<float>& buffer)
    {
        const float outputGain = juce::Decibels::decibelsToGain(volume);
        const float autoGain = juce::Decibels::decibelsToGain(drive / -5.0f) * (-0.7f * anger + 1.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                float wetSample = buffer.getSample(channel, sample);
                wetSample *= (drive / 10.0f) + 1.0f;       // apply drive
                wetSample += offset;                       // apply dc offset
                distortSample(wetSample, distortionType);  // apply distortion
                wetSample *= autoGain;                     // apply autogain
                wetSample *= outputGain;                   // apply volume
                buffer.setSample(channel, sample, wetSample);
            }
        }
    }

    void distortSample(float& sample, int type)
    {
        float angerValue;
        switch (type)
        {
        case 0: // inverse absolute value
            angerValue = -0.9f * anger + 1.0f;   // 1.0 - 0.1
            sample = sample / (angerValue + abs(sample));
            break;
        case 1: // arctan
            angerValue = -2.5f * anger + 3.0f;    // 3.0 - 0.5
            sample = (2.0f / juce::float_Pi) * atan((juce::float_Pi / angerValue) * sample);
            break;
        case 2: // erf
            angerValue = -2.5f * anger + 3.0f;  // 3.0 - 0.5
            sample = erf(sample * sqrt(juce::float_Pi) / angerValue);
            break;
        case 3: // inverse square root
            angerValue = 4.5f * anger + 0.5f;    // 0.5 - 5.0
            sample = sample / sqrt((1.0f / angerValue) + (sample * sample));
            break;
        }
    }

    void applyMix(juce::dsp::AudioBlock<float>& wet, const juce::AudioBuffer<float>& dry)
    {
        // sin6dB
        const float dryMix = std::pow(std::sin(0.5f * juce::float_Pi * (1.0f - mix)), 2.0f);
        const float wetMix = std::pow(std::sin(0.5f * juce::float_Pi * mix), 2.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float wetSample = wet.getSample(channel, sample) * wetMix;
                const float drySample = dry.getSample(channel, sample) * dryMix;
                wet.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    int getOversamplerLatency()
    {
        return static_cast<int>(oversampler.getLatencyInSamples());
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float drive{ 0.0f };
    float volume{ 0.0f };
    float offset{ 0.0f };
    float mix{ 0.0f };
    float anger{ 0.5f };
    int distortionType{ 0 };
    float hpfFreq{ 20.0f };
    float lpfFreq{ 20000.0f };
    float shape{ 0.0f };
    bool shapeTilt{ true };
    juce::AudioBuffer<float> dryBuffer;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
        juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter> filterChain;
    enum FilterChainIndex
    {
        HPF, LPF, LowShelf, HighShelf
    };
    StereoFilter dcFilter;
    juce::dsp::Oversampling<float> oversampler{ 2, 2, 
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};