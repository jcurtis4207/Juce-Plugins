/*
*   Reverb Module
*       by Jacob Curtis
* 
*   Includes modulation, predelay, and post-filters
*
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

class Reverb
{
public:
    Reverb() 
    {
        // reset delayline to use constructor with maximum delay argument
        processChain.get<ChainIndex::Delay>() = juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>(44100);
    }

    void setParameters(const juce::AudioProcessorValueTreeState& apvts)
    {
        roomSize = apvts.getRawParameterValue("roomSize")->load() * 0.01f;
        damping = apvts.getRawParameterValue("damping")->load() * 0.01f;
        mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        predelay = apvts.getRawParameterValue("predelay")->load();
        modDepth = apvts.getRawParameterValue("modDepth")->load() * 0.005f;
        modRate = apvts.getRawParameterValue("modRate")->load();
        hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        dryBuffer.setSize(numOutputs, bufferSize);
        wetBuffer.setSize(numOutputs, bufferSize);
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        processChain.prepare(spec);
    }

    void process(juce::AudioBuffer<float>& inputBuffer)
    {
        // copy input into buffers
        for (int channel = 0; channel < numOutputs; channel++)
        {
            dryBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
            wetBuffer.copyFrom(channel, 0, inputBuffer.getReadPointer(channel), bufferSize);
        }
        // setup process chain
        setupDelay();
        setupFilters();
        setupModulation();
        setupReverb();
        // apply processing to wet buffer
        wetBuffer.applyGain(0.5f);
        juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
        processChain.process(wetContext);
        applyMix();
        // copy mixed wet buffer to output
        for (int channel = 0; channel < numOutputs; channel++)
        {
            inputBuffer.copyFrom(channel, 0, wetBuffer.getReadPointer(channel), bufferSize);
        }
    }

private:
    void setupDelay()
    {
        processChain.get<ChainIndex::Delay>().setDelay(static_cast<float>(predelay * sampleRate * 0.001f));
    }

    void setupFilters()
    {
        *processChain.get<ChainIndex::HPF>().state = *juce::dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2)[0];
        *processChain.get<ChainIndex::LPF>().state = *juce::dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2)[0];
    }

    void setupModulation()
    {
        auto& chorus = processChain.get<ChainIndex::Chorus>();
        chorus.setCentreDelay(1.0f);
        chorus.setFeedback(0.0f);
        chorus.setMix(1.0f);
        chorus.setDepth(modDepth);
        chorus.setRate(modRate);
    }

    void setupReverb()
    {
        reverbParameters.roomSize = roomSize;
        reverbParameters.damping = damping;
        reverbParameters.width = 1.0f;
        reverbParameters.freezeMode = 0.0f;
        reverbParameters.wetLevel = 1.0f;
        reverbParameters.dryLevel = 0.0f;
        processChain.get<ChainIndex::Verb>().setParameters(reverbParameters);
    }

    // mix wet and dry buffers into wet buffer
    void applyMix()
    {
        // sin6dB
        const float dryMix = std::pow(std::sin(0.5f * juce::float_Pi * (1.0f - mix)), 2.0f);
        const float wetMix = std::pow(std::sin(0.5f * juce::float_Pi * mix), 2.0f);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                const float wetSample = wetBuffer.getSample(channel, sample) * wetMix;
                wetBuffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    float roomSize{ 0.0f };
    float damping{ 0.0f };
    float mix{ 0.0f };
    float predelay{ 0.0f };
    float modDepth{ 0.0f };
    float modRate{ 0.0f };
    float hpfFreq{ 0.0f };
    float lpfFreq{ 0.0f };
    juce::AudioBuffer<float> dryBuffer, wetBuffer;
    juce::Reverb::Parameters reverbParameters;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain < juce::dsp::Chorus<float>, juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>,
        juce::dsp::Reverb, StereoFilter, StereoFilter> processChain;
    enum ChainIndex {
        Chorus, Delay, Verb, HPF, LPF
    };
};