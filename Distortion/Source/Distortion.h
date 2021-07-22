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

class Distortion
{
public:
    Distortion(){}
    ~Distortion(){}

    void setParameters(juce::AudioProcessorValueTreeState& apvts)
    {
        drive = apvts.getRawParameterValue("drive")->load();
        volume = apvts.getRawParameterValue("volume")->load();
        mix = apvts.getRawParameterValue("mix")->load();
        anger = apvts.getRawParameterValue("anger")->load();
        distortionType = static_cast<int>(apvts.getRawParameterValue("type")->load());
        hpfFreq = apvts.getRawParameterValue("hpf")->load();
        lpfFreq = apvts.getRawParameterValue("lpf")->load();
        shape = apvts.getRawParameterValue("shape")->load();
        shapeTilt = apvts.getRawParameterValue("shapeTilt")->load();
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate * 4.0;
        bufferSize = maxBlockSize * 4;
        dryBuffer.setSize(numChannels, bufferSize);
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numChannels;
        // prepare the process chain
        filterChain.prepare(spec);
        // setup oversampling
        oversampler.reset();
        oversampler.initProcessing(bufferSize);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        // create blocks
        auto upBlock = oversampler.processSamplesUp(context.getInputBlock());
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(0), upBlock.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(1), upBlock.getChannelPointer(1), bufferSize);
        // apply filters, distortion, and mix
        applyFilters(upBlock);
        distortBuffer(upBlock);
        applyMix(upBlock, dryBuffer);
        oversampler.processSamplesDown(context.getOutputBlock());
    }

    void applyFilters(juce::dsp::AudioBlock<float>& buffer)
    {
        // set coefficients from parameters
        auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2);
        auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2);
        auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 900.0f, 0.4f, juce::Decibels::decibelsToGain(shape * -1.0f));
        auto highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 900.0f, 0.4f, juce::Decibels::decibelsToGain(shape));
        // apply coefficients to filters
        *filterChain.get<FilterChainIndex::HPF>().state = *hpfCoefficients[0];
        *filterChain.get<FilterChainIndex::LPF>().state = *lpfCoefficients[0];
        *filterChain.get<FilterChainIndex::LowShelf>().state = *lowShelfCoefficients;
        *filterChain.get<FilterChainIndex::HighShelf>().state = *highShelfCoefficients;
        // bypass high shelf if tilt disabled
        filterChain.setBypassed<FilterChainIndex::HighShelf>(!shapeTilt);
        // apply processing
        juce::dsp::AudioBlock<float> filterBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

    void distortBuffer(juce::dsp::AudioBlock<float>& buffer)
    {
        float outputGain = juce::Decibels::decibelsToGain(volume);
        float autoGain = juce::Decibels::decibelsToGain(drive / -5.0f) * (-0.7f * anger + 1.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            float wetSample[2] = { buffer.getSample(0, sample), buffer.getSample(1, sample) };
            for (int channel = 0; channel < 2; channel++)
            {
                wetSample[channel] *= (drive / 10.0f) + 1.0f;       // apply drive
                distortSample(wetSample[channel], distortionType);  // apply distortion
                wetSample[channel] *= autoGain;                     // apply autogain
                wetSample[channel] *= outputGain;                   // apply volume
            }
            buffer.setSample(0, sample, wetSample[0]);
            buffer.setSample(1, sample, wetSample[1]);
        }
    }

    void distortSample(float& sample, int type)
    {
        float angerValue;
        switch (type)
        {
        case 0: // inv abs
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
        case 3: // inv square
            angerValue = 4.5f * anger + 0.5f;    // 0.5 - 5.0
            sample = sample / sqrt((1.0f / angerValue) + (sample * sample));
            break;
        }
    }

    void applyMix(juce::dsp::AudioBlock<float>& wetBuffer, juce::AudioBuffer<float> dry)
    {
        float dryMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (1.0 - (mix / 100.0f))), 2.0));
        float wetMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (mix / 100.0f)), 2.0));
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // calculate mix
            float wetSampleLeft = wetBuffer.getSample(0, sample) * wetMix;
            float wetSampleRight = wetBuffer.getSample(1, sample) * wetMix;
            float drySampleLeft = dry.getSample(0, sample) * dryMix;
            float drySampleRight = dry.getSample(1, sample) * dryMix;
            // output mixed signal
            wetBuffer.setSample(0, sample, wetSampleLeft + drySampleLeft);
            wetBuffer.setSample(1, sample, wetSampleRight + drySampleRight);
        }
    }

    const int getOversamplerLatency()
    {
        return (int)oversampler.getLatencyInSamples();
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float drive{ 0.0f };
    float volume{ 0.0f };
    float mix{ 0.0f };
    float anger{ 0.5f };
    int distortionType{ 0 };
    float hpfFreq{ 20.0f };
    float lpfFreq{ 20000.0f };
    float shape{ 0.0f };
    bool shapeTilt{ true };
    juce::AudioBuffer<float> dryBuffer;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter> filterChain;
    enum FilterChainIndex
    {
        HPF, LPF, LowShelf, HighShelf
    };
    juce::dsp::Oversampling<float> oversampler{ 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false, true };
};