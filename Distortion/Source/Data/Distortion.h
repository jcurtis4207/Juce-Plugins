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
        offset = apvts.getRawParameterValue("offset")->load();
        anger = apvts.getRawParameterValue("anger")->load();
        distortionType = static_cast<int>(apvts.getRawParameterValue("type")->load());
        hpfFreq = apvts.getRawParameterValue("hpf")->load();
        lpfFreq = apvts.getRawParameterValue("lpf")->load();
        shape = apvts.getRawParameterValue("shape")->load();
        shapeTilt = apvts.getRawParameterValue("shapeTilt")->load();
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = static_cast<float>(newSampleRate);
        distortionBuffer.setSize(numChannels, maxBlockSize);
        dryBuffer.setSize(numChannels, maxBlockSize);
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = newSampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numChannels;
        // prepare the process chain
        filterChain.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy input to distortion and dry buffers
        juce::FloatVectorOperations::copy(distortionBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(distortionBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(dryBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // apply filters, distortion, and mix
        applyFilters();  
        distortBuffer(distortionBuffer.getWritePointer(0), distortionBuffer.getWritePointer(1), bufferSize);
        applyMix(distortionBuffer.getWritePointer(0), distortionBuffer.getWritePointer(1), dryBuffer.getReadPointer(0), dryBuffer.getReadPointer(1), bufferSize);
        // copy distortion buffer to output
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), distortionBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), distortionBuffer.getReadPointer(1), bufferSize);
    }

    void applyFilters()
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
        juce::dsp::AudioBlock<float> filterBlock(distortionBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

    void distortBuffer(float* bufferLeft, float* bufferRight, int bufferSize)
    {
        float outputGain = juce::Decibels::decibelsToGain(volume);
        float autoGain = juce::Decibels::decibelsToGain(drive / -5.0f) * (-0.7f * anger + 1.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            float wetSample[2] = { bufferLeft[sample], bufferRight[sample] };
            for (int channel = 0; channel < 2; channel++)
            {
                wetSample[channel] *= (drive / 10.0f) + 1.0f;       // apply drive
                wetSample[channel] += offset;                       // apply dc offset
                distortSample(wetSample[channel], distortionType);  // apply distortion
                wetSample[channel] *= autoGain;                     // apply autogain
                wetSample[channel] *= outputGain;                   // apply volume
            }
            // set wet to output
            bufferLeft[sample] = wetSample[0];
            bufferRight[sample] = wetSample[1];
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

    void applyMix(float* wetLeft, float* wetRight, const float* dryLeft, const float* dryRight, int bufferSize)
    {
        float dryMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (1.0 - (mix / 100.0f))), 2.0));
        float wetMix = static_cast<float> (std::pow(std::sin(0.5 * juce::MathConstants<double>::pi * (mix / 100.0f)), 2.0));
        for (int sample = 0; sample < bufferSize; sample++)
        {
            // calculate mix
            float wetSampleLeft = wetLeft[sample] * wetMix;
            float wetSampleRight = wetRight[sample] * wetMix;
            float drySampleLeft = dryLeft[sample] * dryMix;
            float drySampleRight = dryRight[sample] * dryMix;
            // output mix to wet
            wetLeft[sample] = wetSampleLeft + drySampleLeft;
            wetRight[sample] = wetSampleRight + drySampleRight;
        }
    }

private:
    float sampleRate{ 0.0f };
    float drive{ 0.0f };
    float volume{ 0.0f };
    float mix{ 0.0f };
    float offset{ 0.0f };
    float anger{ 0.5f };
    int distortionType{ 0 };
    float hpfFreq{ 20.0f };
    float lpfFreq{ 20000.0f };
    float shape{ 0.0f };
    bool shapeTilt{ true };
    juce::AudioBuffer<float> distortionBuffer, dryBuffer;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter> filterChain;
    enum FilterChainIndex
    {
        HPF, LPF, LowShelf, HighShelf
    };
};