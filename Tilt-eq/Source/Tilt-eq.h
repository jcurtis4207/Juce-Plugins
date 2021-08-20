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

	void setParameters(const juce::AudioProcessorValueTreeState& apvts)
	{
		freq = apvts.getRawParameterValue("freq")->load();
		gain = apvts.getRawParameterValue("gain")->load();
	}

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = newSampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = 2;
        filterChain.prepare(spec);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // create shelves with fixed 0.4Q
        *filterChain.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, freq, 0.4f, juce::Decibels::decibelsToGain(gain * -1.0f));
        *filterChain.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, freq, 0.4f, juce::Decibels::decibelsToGain(gain));
        // apply processing to buffer
        juce::dsp::AudioBlock<float> filterBlock(inputBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        filterChain.process(filterContext);
    }

private:
    double sampleRate{ 0.0 };
	float freq{ 1000.0f };
	float gain{ 0.0f };
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
        juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};