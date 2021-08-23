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

#define numOutputs 2

class Gate
{
public:
	void setParameters(const juce::AudioProcessorValueTreeState& apvts, bool isListen)
	{
		threshold = apvts.getRawParameterValue("threshold")->load();
		ratio = apvts.getRawParameterValue("ratio")->load();
		const float inputAttack = apvts.getRawParameterValue("attack")->load() / 1000.0f;
		attackTime = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate * inputAttack)));
		const float inputRelease = apvts.getRawParameterValue("release")->load() / 1000.0f;
		releaseTime = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate * inputRelease)));
		const float inputHold = apvts.getRawParameterValue("hold")->load();
		holdTime = static_cast<int>(round(inputHold * 0.001f * sampleRate));
		hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
		lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
		filterEnable = apvts.getRawParameterValue("filterEnable")->load();
		listen = isListen;
	}

	void prepare(double newSampleRate, int maxBlockSize)
	{
		sampleRate = newSampleRate;
		bufferSize = maxBlockSize;
		gainReductionBuffer.setSize(numOutputs, maxBlockSize);
		sideChainBuffer.setSize(numOutputs, maxBlockSize);
		juce::dsp::ProcessSpec spec;
		spec.sampleRate = newSampleRate;
		spec.maximumBlockSize = maxBlockSize;
		spec.numChannels = numOutputs;
		filterChain.prepare(spec);
	}

	void process(juce::AudioBuffer<float>& inputBuffer)
	{
		// copy the buffer into the sidechain and gr buffer
		for (int channel = 0; channel < numOutputs; channel++)
		{
			sideChainBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
			gainReductionBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
		}
		// if listen set, apply filters and output sidechain
		if (listen)
		{
			applyFilters();
			for (int channel = 0; channel < numOutputs; channel++)
			{
				inputBuffer.copyFrom(channel, 0, sideChainBuffer.getReadPointer(channel), bufferSize);
			}
			// reset gain reduction
			outputGainReduction = { 1.0f, 1.0f };
		}
		else
		{
			if (filterEnable)
			{
				applyFilters();
			}
			// get gain reduction from sidechain and multiply to output
			applyGate();
			for (int channel = 0; channel < numOutputs; channel++)
			{
				juce::FloatVectorOperations::multiply(inputBuffer.getWritePointer(channel),
					gainReductionBuffer.getReadPointer(channel), bufferSize);
			}
		}
	}

	float getGainReductionLeft()
	{
		return juce::Decibels::gainToDecibels(outputGainReduction[0]) * -1.0f;
	}

	float getGainReductionRight()
	{
		return juce::Decibels::gainToDecibels(outputGainReduction[1]) * -1.0f;
	}

private:
	// apply hpf and lpf to sidechain buffer
	void applyFilters()
	{
		*filterChain.get<0>().state = *juce::dsp::FilterDesign<float>::
			designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2)[0];
		*filterChain.get<1>().state = *juce::dsp::FilterDesign<float>::
			designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2)[0];
		juce::dsp::AudioBlock<float> filterBlock(sideChainBuffer);
		juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
		filterChain.process(filterContext);
	}

	// use sidechain to calculate gain reduction multiplier on gr buffer
	void applyGate()
	{
		for (int sample = 0; sample < bufferSize; sample++)
		{
			outputGainReduction = { 0.0f, 0.0f };
			for (int channel = 0; channel < numOutputs; channel++)
			{
				const float inputSample = juce::Decibels::gainToDecibels(
					sideChainBuffer.getSample(channel, sample));
				// negative -> under threshold, positive -> over threshold
				const float levelVsThreshold = inputSample - threshold;
				// attenuation needed to get to target level
				const float attenuation = juce::Decibels::decibelsToGain(
					levelVsThreshold * (ratio - 1));
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
				outputGainReduction[channel] = (currentState[channel] > outputGainReduction[channel]) ?
					currentState[channel] : outputGainReduction[channel];
				// output multiplier to gr buffer
				gainReductionBuffer.setSample(channel, sample, currentState[channel]);
			}
		}
	}

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
	std::array<float, numOutputs> currentState{ 0.0f, 0.0f };
	std::array<int, numOutputs> currentHold{ 0, 0 };
	std::array<float, numOutputs> outputGainReduction{ 1.0f, 1.0f };
	using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
		juce::dsp::IIR::Coefficients<float>>;
	juce::dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};