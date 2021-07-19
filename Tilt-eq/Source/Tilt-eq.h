/*
*   Tilt-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>

class Tilteq
{
public:
	Tilteq(){}
	~Tilteq(){}

	void setParameters(juce::AudioProcessorValueTreeState& apvts)
	{
		freq = apvts.getRawParameterValue("freq")->load();
		gain = apvts.getRawParameterValue("gain")->load();
	}

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
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
        // create shelves with fixed 0.4Q
        auto lowShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, freq, 0.4f, juce::Decibels::decibelsToGain(gain * -1.0f));
        auto highShelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freq, 0.4f, juce::Decibels::decibelsToGain(gain));
        // apply coefficients to filters
        *filterChain.get<0>().state = *lowShelfCoefficients;
        *filterChain.get<1>().state = *highShelfCoefficients;
        // apply processing to output buffer
        juce::dsp::AudioBlock<float> filterBlock(outputBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

private:
    double sampleRate{ 0.0 };
	float freq{ 1000.0f };
	float gain{ 0.0f };
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};