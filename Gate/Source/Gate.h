/*
*   Gate Module
*       by Jacob Curtis
*
*   Originally inspired by Daniel Rudrich's "Simple Compressor"
*   https://github.com/DanielRudrich/SimpleCompressor
*
*/

#pragma once
#include <JuceHeader.h>

class Gate
{
public:
	Gate() {}

	~Gate() {}

	void setParameters(juce::AudioProcessorValueTreeState& apvts, bool isListen)
	{
		threshold = apvts.getRawParameterValue("threshold")->load();
		ratio = apvts.getRawParameterValue("ratio")->load();
		float inputAttack = apvts.getRawParameterValue("attack")->load() / 1000.0f;
		float inputRelease = apvts.getRawParameterValue("release")->load() / 1000.0f;
		float inputHold = apvts.getRawParameterValue("hold")->load();
		attackTime = 1.0f - std::exp(-1.0f / ((float)sampleRate * inputAttack));
		releaseTime = 1.0f - std::exp(-1.0f / ((float)sampleRate * inputRelease));
		holdTime = (int)round((inputHold * 0.001f) * sampleRate);
		hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
		lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
		filterEnable = apvts.getRawParameterValue("filterEnable")->load();
		listen = isListen;
	}

	void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
	{
		sampleRate = newSampleRate;
		bufferSize = maxBlockSize;
		// initialize buffers
		gainReductionBuffer.setSize(numChannels, maxBlockSize);
		sideChainBuffer.setSize(numChannels, maxBlockSize);
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
		// copy the buffer into the sidechain and gr buffer
		for (int channel = 0; channel < 2; channel++)
		{
			juce::FloatVectorOperations::copy(sideChainBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
			juce::FloatVectorOperations::copy(gainReductionBuffer.getWritePointer(channel), outputBuffer.getChannelPointer(channel), bufferSize);
		}
		// if listen set, apply filters and output sidechain
		if (listen)
		{
			applyFilters();
			juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), sideChainBuffer.getReadPointer(0), bufferSize);
			juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), sideChainBuffer.getReadPointer(1), bufferSize);
			// reset gain reduction
			outputGainReduction[0] = 1.0f;
			outputGainReduction[1] = 1.0f;
		}
		else
		{
			if (filterEnable)
			{
				applyFilters();
			}
			// get gain reduction from sidechain and multiply to output
			applyGate();
			juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(0), gainReductionBuffer.getReadPointer(0), bufferSize);
			juce::FloatVectorOperations::multiply(outputBuffer.getChannelPointer(1), gainReductionBuffer.getReadPointer(1), bufferSize);
		}
	}

	// apply hpf and lpf to sidechain buffer
	void applyFilters()
	{
		// set coefficients from parameters
		auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2);
		auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2);
		// apply coefficients to filters
		*filterChain.get<0>().state = *hpfCoefficients[0];
		*filterChain.get<1>().state = *lpfCoefficients[0];
		// apply processing
		juce::dsp::AudioBlock<float> filterBlock(sideChainBuffer);
		juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
		filterChain.process(filterContext);
	}

	// use sidechain to calculate gain reduction multiplier on gr buffer
	void applyGate()
	{
		for (int sample = 0; sample < bufferSize; sample++)
		{
			outputGainReduction[0] = 0.0f;
			outputGainReduction[1] = 0.0f;
			for (int channel = 0; channel < 2; channel++)
			{
				// get sample from side chain
				const float inputSample = juce::Decibels::gainToDecibels(sideChainBuffer.getSample(channel, sample));
				// negative -> under threshold, positive -> over threshold
				const float levelVsThreshold = inputSample - threshold;
				// attenuation needed to get to target level
				const float attenuation = juce::Decibels::decibelsToGain(levelVsThreshold * (ratio - 1));
				// target gain reduction multiplier - none needed if level exceeds threshold
				const float targetState = (levelVsThreshold >= 0.0f) ? 1.0f : attenuation;
				// get current gate state compared to target gate state
				const float stateDifference = targetState - currentState[channel];
				// apply ballistics
				if (stateDifference > 0.0f) // gate opening -> attack
				{
					currentState[channel] += attackTime * stateDifference;
					currentHold[channel] = 0;	// reset hold
				}
				else // gate closing
				{
					if (currentHold[channel] < holdTime) // hold - dont change state until hold completes
					{
						currentHold[channel]++;
					}
					else // release
					{
						currentState[channel] += releaseTime * stateDifference;
					}
				}
				// set gain reduction for meter
				outputGainReduction[channel] = (currentState[channel] > outputGainReduction[channel]) ? currentState[channel] : outputGainReduction[channel];
				// output multiplier to gr buffer
				gainReductionBuffer.setSample(channel, sample, currentState[channel]);
			}
		}
	}

	const float getGainReductionLeft()
	{
		return juce::Decibels::gainToDecibels(outputGainReduction[0]) * -1.0f;
	}

	const float getGainReductionRight()
	{
		return juce::Decibels::gainToDecibels(outputGainReduction[1]) * -1.0f;
	}

private:
	double sampleRate{ 0.0 };
	int bufferSize{ 0 };
	float threshold{ 0.0f };
	float ratio{ 4.0f };
	float attackTime{ 0.0f };
	float releaseTime{ 10.0f };
	int holdTime{ 0 };
	float hpfFreq{ 20.0f };
	float lpfFreq{ 20000.0f };
	bool filterEnable{ false };
	bool listen{ false };
	juce::AudioBuffer<float> gainReductionBuffer, sideChainBuffer;
	float currentState[2]{ 0.0f, 0.0f };
	int currentHold[2]{ 0, 0 };
	float outputGainReduction[2]{ 1.0f, 1.0f };
	using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
	juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};