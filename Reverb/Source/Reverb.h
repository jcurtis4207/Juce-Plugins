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

using namespace juce;

struct Params {
    float roomSize, damping, mix, predelay, 
        modDepth, modRate, hpfFreq, lpfFreq;
};

class ReverbModule
{
public:
    ReverbModule()
    {
        // reset delayline to use constructor with maximum delay argument
        processChain.get<ChainIndex::Delay>() = 
            dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear>(44100);
    }

    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.roomSize = apvts.getRawParameterValue("roomSize")->load() * 0.01f;
        parameters.damping = apvts.getRawParameterValue("damping")->load() * 0.01f;
        parameters.mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
        parameters.predelay = apvts.getRawParameterValue("predelay")->load();
        parameters.modDepth = apvts.getRawParameterValue("modDepth")->load() * 0.005f;
        parameters.modRate = apvts.getRawParameterValue("modRate")->load();
        parameters.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        parameters.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
    }

    void prepare(double inputSampleRate, int maxBlockSize)
    {
        sampleRate = inputSampleRate;
        bufferSize = maxBlockSize;
        dryBuffer.setSize(numOutputs, bufferSize);
        wetBuffer.setSize(numOutputs, bufferSize);
        dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = bufferSize;
        spec.numChannels = numOutputs;
        processChain.prepare(spec);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        dryBuffer.makeCopyOf(inputBuffer, true);
        wetBuffer.makeCopyOf(inputBuffer, true);
        setupDelay();
        setupFilters();
        setupModulation();
        setupReverb();
        wetBuffer.applyGain(0.5f); // reverb is loud
        dsp::AudioBlock<float> wetBlock(wetBuffer);
        dsp::ProcessContextReplacing<float> wetContext(wetBlock);
        processChain.process(wetContext);
        mixToOutput(inputBuffer);
    }

private:
    void setupDelay()
    {
        processChain.get<ChainIndex::Delay>().setDelay(
            static_cast<float>(parameters.predelay * sampleRate * 0.001f));
    }

    void setupFilters()
    {
        *processChain.get<ChainIndex::HPF>().state = *dsp::FilterDesign<float>::
            designIIRHighpassHighOrderButterworthMethod(parameters.hpfFreq, sampleRate, 2)[0];
        *processChain.get<ChainIndex::LPF>().state = *dsp::FilterDesign<float>::
            designIIRLowpassHighOrderButterworthMethod(parameters.lpfFreq, sampleRate, 2)[0];
    }

    void setupModulation()
    {
        processChain.get<ChainIndex::Chorus>().setCentreDelay(1.0f);
        processChain.get<ChainIndex::Chorus>().setFeedback(0.0f);
        processChain.get<ChainIndex::Chorus>().setMix(1.0f);
        processChain.get<ChainIndex::Chorus>().setDepth(parameters.modDepth);
        processChain.get<ChainIndex::Chorus>().setRate(parameters.modRate);
    }

    void setupReverb()
    {
        reverbParameters.roomSize = parameters.roomSize;
        reverbParameters.damping = parameters.damping;
        reverbParameters.width = 1.0f;
        reverbParameters.freezeMode = 0.0f;
        reverbParameters.wetLevel = 1.0f;
        reverbParameters.dryLevel = 0.0f;
        processChain.get<ChainIndex::Verb>().setParameters(reverbParameters);
    }

    void mixToOutput(AudioBuffer<float>& buffer)
    {
        const float dryMix = std::sin(0.5f * float_Pi * (1.0f - parameters.mix));
        const float wetMix = std::sin(0.5f * float_Pi * parameters.mix);
        for (int sample = 0; sample < bufferSize; sample++)
        {
            for (int channel = 0; channel < numOutputs; channel++)
            {
                const float drySample = dryBuffer.getSample(channel, sample) * dryMix;
                const float wetSample = wetBuffer.getSample(channel, sample) * wetMix;
                buffer.setSample(channel, sample, wetSample + drySample);
            }
        }
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    AudioBuffer<float> dryBuffer, wetBuffer;
    Params parameters;
    juce::Reverb::Parameters reverbParameters;
    using StereoFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>;
    dsp::ProcessorChain < dsp::Chorus<float>, dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::Linear>,
        dsp::Reverb, StereoFilter, StereoFilter> processChain;
    enum ChainIndex { Chorus, Delay, Verb, HPF, LPF };
};