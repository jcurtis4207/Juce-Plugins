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

using namespace juce;

struct Parameters {
	float threshold, ratio, attackTime, releaseTime, hpfFreq, lpfFreq;
	int holdTime;
	bool filterEnable, listen;
};

class Gate
{
public:
	void setParameters(const AudioProcessorValueTreeState& apvts, bool isListen)
	{
		parameters.threshold = apvts.getRawParameterValue("threshold")->load();
		parameters.ratio = apvts.getRawParameterValue("ratio")->load();
		const float inputAttack = apvts.getRawParameterValue("attack")->load() / 1000.0f;
		parameters.attackTime = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate * inputAttack)));
		const float inputRelease = apvts.getRawParameterValue("release")->load() / 1000.0f;
		parameters.releaseTime = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate * inputRelease)));
		const float inputHold = apvts.getRawParameterValue("hold")->load();
		parameters.holdTime = static_cast<int>(round(inputHold * sampleRate * 0.001f));
		parameters.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
		parameters.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
		parameters.filterEnable = apvts.getRawParameterValue("filterEnable")->load();
		parameters.listen = isListen;
	}

	void prepare(double newSampleRate, int maxBlockSize)
	{
		sampleRate = newSampleRate;
		bufferSize = maxBlockSize;
		gainReductionBuffer.setSize(numOutputs, maxBlockSize);
		sideChainBuffer.setSize(numOutputs, maxBlockSize);
		dsp::ProcessSpec spec;
		spec.sampleRate = newSampleRate;
		spec.maximumBlockSize = maxBlockSize;
		spec.numChannels = numOutputs;
		filterChain.prepare(spec);
	}

	void process(AudioBuffer<float>& inputBuffer)
	{
		sideChainBuffer.makeCopyOf(inputBuffer, true);
		gainReductionBuffer.makeCopyOf(inputBuffer, true);
		if (parameters.listen)
		{
			applyListen(inputBuffer);
			return;
		}
		if (parameters.filterEnable)
		{
			applyFiltersToSideChain();
		}
		calculateGainReduction();
		applyGainReduction(inputBuffer);
	}

	std::array<float, numOutputs> getGainReduction()
	{
		return std::array<float, numOutputs> {
			Decibels::gainToDecibels(outputGainReduction[0]) * -1.0f,
			Decibels::gainToDecibels(outputGainReduction[1]) * -1.0f,
		};
	}

private:
	void applyFiltersToSideChain()
	{
		*filterChain.get<0>().state = *dsp::FilterDesign<float>::
			designIIRHighpassHighOrderButterworthMethod(parameters.hpfFreq, sampleRate, 2)[0];
		*filterChain.get<1>().state = *dsp::FilterDesign<float>::
			designIIRLowpassHighOrderButterworthMethod(parameters.lpfFreq, sampleRate, 2)[0];
		dsp::AudioBlock<float> filterBlock(sideChainBuffer);
		dsp::ProcessContextReplacing<float> filterContext(filterBlock);
		filterChain.process(filterContext);
	}

	void applyListen(AudioBuffer<float>& buffer)
	{
		applyFiltersToSideChain();
		buffer.makeCopyOf(sideChainBuffer, true);
		outputGainReduction = { 1.0f, 1.0f };
	}

	void applyHisteresis(float newMultiplier, int channel)
	{
		if (newMultiplier > currentMultiplier[channel]) // gate opening -> attack
		{
			currentMultiplier[channel] += parameters.attackTime * (newMultiplier - currentMultiplier[channel]);
			currentHold[channel] = 0;
		}
		else // gate closing
		{
			if (currentHold[channel] < parameters.holdTime) // -> hold
			{
				currentHold[channel]++;
			}
			else // -> release
			{
				currentMultiplier[channel] += parameters.releaseTime * (newMultiplier - currentMultiplier[channel]);
			}
		}
	}

	void calculateGainReduction()
	{
		for (int sample = 0; sample < bufferSize; sample++)
		{
			outputGainReduction = { 0.0f, 0.0f };
			for (int channel = 0; channel < numOutputs; channel++)
			{
				const float inputSample = Decibels::gainToDecibels(
					sideChainBuffer.getSample(channel, sample));
				const float levelUnderThreshold = jmin(inputSample - parameters.threshold, 0.0f);
				const float newMultiplier = Decibels::decibelsToGain(
					levelUnderThreshold * (parameters.ratio - 1));
				applyHisteresis(newMultiplier, channel);
				outputGainReduction[channel] = jmax(
					currentMultiplier[channel], outputGainReduction[channel]);
				gainReductionBuffer.setSample(channel, sample, currentMultiplier[channel]);
			}
		}
	}

	void applyGainReduction(AudioBuffer<float>& buffer)
	{
		for (int channel = 0; channel < numOutputs; channel++)
		{
			FloatVectorOperations::multiply(buffer.getWritePointer(channel),
				gainReductionBuffer.getReadPointer(channel), bufferSize);
		}
	}

	double sampleRate{ 0.0 };
	int bufferSize{ 0 };
	Parameters parameters;
	AudioBuffer<float> gainReductionBuffer, sideChainBuffer;
	std::array<float, numOutputs> currentMultiplier{ 0.0f, 0.0f };
	std::array<int, numOutputs> currentHold{ 0, 0 };
	std::array<float, numOutputs> outputGainReduction{ 1.0f, 1.0f };
	using StereoFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, 
		dsp::IIR::Coefficients<float>>;
	dsp::ProcessorChain<StereoFilter, StereoFilter> filterChain;
};