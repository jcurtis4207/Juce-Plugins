/*
* 
* Dual Filter Module
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
* 
* Highpass and Lowpass filters with 12dB/Oct slope
* Frequency and Bypass parameters
* 
* *** Dual Mono DSP Processor chains need to be setup ***
* 
* In PluginProcessor.h
*   add 2 IIR filter objects to processor chain:
        juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Filter<float>
*
*   create object: 
        DualFilter dualFilter
* 
* In PluginProcessor.cpp
*   in initialization list, add: 
        dualFilter(parameters)
* 
*   in prepareToPlay, after preparing chains, get Filters from chain using int indeces or ChainIndex enum:
        auto& leftHPF = leftChain.get<0>();
        auto& rightHPF = rightChain.get<ChainIndex::HPF>();
        auto& leftLPF = leftChain.get<1>();
        auto& rightLPF = rightChain.get<ChainIndex::LPF>();
        dualFilter.updateFilters(parameters, sampleRate, leftHPF, rightHPF, leftLPF, rightLPF);

*   in ProcessBlock, before initializing dsp audio blocks, get Filters from chain using int indeces or ChainIndex enum:
        auto& leftHPF = leftChain.get<0>();
        auto& rightHPF = rightChain.get<ChainIndex::HPF>();
        auto& leftLPF = leftChain.get<1>();
        auto& rightLPF = rightChain.get<ChainIndex::LPF>();
        dualFilter.updateFilters(parameters, getSampleRate(), leftHPF, rightHPF, leftLPF, rightLPF);
* 
*/

#pragma once
#include <JuceHeader.h>

class DualFilter
{
public:
    DualFilter(juce::AudioProcessorValueTreeState& apvts)
    {
        // create filter parameters
        apvts.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("hpfFreq", "HPF Frequency", juce::NormalisableRange<float>{20.0f, 20000.0f, 1.0f, 0.25f}, 20.0f));
        apvts.createAndAddParameter(std::make_unique<juce::AudioParameterFloat>("lpfFreq", "LPF Frequency", juce::NormalisableRange<float>{20.0f, 20000.0f, 1.0f, 0.25f}, 20000.0f));
        apvts.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("hpfBypass", "HPF Bypass", false));
        apvts.createAndAddParameter(std::make_unique<juce::AudioParameterBool>("lpfBypass", "LPF Bypass", false));
    }

    ~DualFilter()
    {
    }

    void updateFilters(juce::AudioProcessorValueTreeState& apvts, double sampleRate,
        juce::dsp::IIR::Filter<float>& leftHPF, juce::dsp::IIR::Filter<float>& rightHPF,
        juce::dsp::IIR::Filter<float>& leftLPF, juce::dsp::IIR::Filter<float>& rightLPF)
    {
        // get parameter values
        float hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        float lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        bool hpfBypass = apvts.getRawParameterValue("hpfBypass")->load();
        bool lpfBypass = apvts.getRawParameterValue("lpfBypass")->load();
        // bypass hpf
        if (hpfBypass)
        {
            *leftHPF.coefficients = bypassCoefficients;
            *rightHPF.coefficients = bypassCoefficients;
        }
        // update hpf coefficients
        else
        {
            auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(hpfFreq, sampleRate, 2);
            *leftHPF.coefficients = *hpfCoefficients[0];
            *rightHPF.coefficients = *hpfCoefficients[0];
        }
        // bypass lpf
        if (lpfBypass)
        {
            *leftLPF.coefficients = bypassCoefficients;
            *rightLPF.coefficients = bypassCoefficients;
        }
        // update lpf coefficients
        else
        {
            auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(lpfFreq, sampleRate, 2);
            *leftLPF.coefficients = *lpfCoefficients[0];
            *rightLPF.coefficients = *lpfCoefficients[0];
        }
    }

private:
    // filter coefficients for bypassing
    juce::dsp::IIR::Coefficients<float> bypassCoefficients{ 1.0f, 0.0f, 1.0f, 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DualFilter)
};
