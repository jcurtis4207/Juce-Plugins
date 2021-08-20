/*
*   E-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>

#define numOutputs 2

struct ParameterValues
{
    bool band1Bell{ false }, band4Bell{ false },
        hpfBypass{ false }, lpfBypass{ false };
    float hpfFreq{ 0 }, lpfFreq{ 0 },
        band1Freq{ 0 }, band1Gain{ 0 }, band1Q{ 0 },
        band2Freq{ 0 }, band2Gain{ 0 }, band2Q{ 0 },
        band3Freq{ 0 }, band3Gain{ 0 }, band3Q{ 0 },
        band4Freq{ 0 }, band4Gain{ 0 }, band4Q{ 0 };
    int hpfSlope{ 0 }, lpfSlope{ 0 };
};

class Equalizer
{
public:
    Equalizer() {}
    ~Equalizer() {}

    // populate settings struct from parameters
    ParameterValues getParameterValues(const juce::AudioProcessorValueTreeState& apvts)
    {
        ParameterValues values;
        values.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        values.hpfSlope = static_cast<int>(apvts.getRawParameterValue("hpfSlope")->load());
        values.hpfBypass = apvts.getRawParameterValue("hpfBypass")->load();
        values.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        values.lpfSlope = static_cast<int>(apvts.getRawParameterValue("lpfSlope")->load());
        values.lpfBypass = apvts.getRawParameterValue("lpfBypass")->load();
        values.band1Freq = apvts.getRawParameterValue("band1Freq")->load();
        values.band1Gain = apvts.getRawParameterValue("band1Gain")->load();
        values.band1Q = apvts.getRawParameterValue("band1Q")->load();
        values.band2Freq = apvts.getRawParameterValue("band2Freq")->load();
        values.band2Gain = apvts.getRawParameterValue("band2Gain")->load();
        values.band2Q = apvts.getRawParameterValue("band2Q")->load();
        values.band3Freq = apvts.getRawParameterValue("band3Freq")->load();
        values.band3Gain = apvts.getRawParameterValue("band3Gain")->load();
        values.band3Q = apvts.getRawParameterValue("band3Q")->load();
        values.band4Freq = apvts.getRawParameterValue("band4Freq")->load();
        values.band4Gain = apvts.getRawParameterValue("band4Gain")->load();
        values.band4Q = apvts.getRawParameterValue("band4Q")->load();
        values.band1Bell = apvts.getRawParameterValue("band1Bell")->load();
        values.band4Bell = apvts.getRawParameterValue("band4Bell")->load();
        return values;
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = newSampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numOutputs;
        processChain.prepare(spec);
    }

    void process(juce::AudioBuffer<float>& inputBuffer, const juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameter values and setup coefficients
        const auto parameterValues = getParameterValues(apvts);
        updateFilters(parameterValues);
        updateEqBands(parameterValues);
        // process filters
        juce::dsp::AudioBlock<float> block(inputBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        processChain.process(context);
    }

    // update filter coefficients from settings struct
    void updateFilters(const ParameterValues& inputValues)
    {
        // setup hpf
        if (inputValues.hpfBypass)
        {
            processChain.setBypassed<ChainIndex::HPF>(true);
        }
        else
        {
            processChain.setBypassed<ChainIndex::HPF>(false);
            *processChain.get<ChainIndex::HPF>().state = *juce::dsp::FilterDesign<float>::
                designIIRHighpassHighOrderButterworthMethod(inputValues.hpfFreq, sampleRate, 
                    2 * (inputValues.hpfSlope + 1))[0];
        }
        // setup lpf
        if (inputValues.lpfBypass)
        {
            processChain.setBypassed<ChainIndex::LPF>(true);
        }
        else
        {
            processChain.setBypassed<ChainIndex::LPF>(false);
            *processChain.get<ChainIndex::LPF>().state = *juce::dsp::FilterDesign<float>::
                designIIRLowpassHighOrderButterworthMethod(inputValues.lpfFreq, sampleRate, 
                    2 * (inputValues.lpfSlope + 1))[0];
        }
    }

    // update parametric bands coefficients from settings struct
    void updateEqBands(const ParameterValues& inputValues)
    {
        // apply peak/shelf coefficients for all bands
        *processChain.get<ChainIndex::Band1>().state = (inputValues.band1Bell) ?
            *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band1Freq,
                inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain)) :
            *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, inputValues.band1Freq,
                inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain));;
        *processChain.get<ChainIndex::Band2>().state = *juce::dsp::IIR::Coefficients<float>::
            makePeakFilter(sampleRate, inputValues.band2Freq, inputValues.band2Q, 
                juce::Decibels::decibelsToGain(inputValues.band2Gain));;
        *processChain.get<ChainIndex::Band3>().state = *juce::dsp::IIR::Coefficients<float>::
            makePeakFilter(sampleRate, inputValues.band3Freq, inputValues.band3Q, 
                juce::Decibels::decibelsToGain(inputValues.band3Gain));;
        *processChain.get<ChainIndex::Band4>().state = (inputValues.band4Bell) ?
            *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band4Freq,
                inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain)) :
            *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, inputValues.band4Freq,
                inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain));
    }

private:
    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
        juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter, 
        StereoFilter, StereoFilter> processChain;
    enum ChainIndex
    {
        HPF, LPF, Band1, Band2, Band3, Band4
    };
};