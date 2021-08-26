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

using namespace juce;

struct Parameters
{
    bool band1Bell, band4Bell, hpfBypass, lpfBypass;
    float hpfFreq, lpfFreq,
        band1Freq, band1Gain, band1Q,
        band2Freq, band2Gain, band2Q,
        band3Freq, band3Gain, band3Q,
        band4Freq, band4Gain, band4Q;
    int hpfSlope, lpfSlope;
};

class Equalizer
{
public:
    void setParameters(const AudioProcessorValueTreeState& apvts)
    {
        parameters.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        parameters.hpfSlope = static_cast<int>(apvts.getRawParameterValue("hpfSlope")->load());
        parameters.hpfBypass = apvts.getRawParameterValue("hpfBypass")->load();
        parameters.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        parameters.lpfSlope = static_cast<int>(apvts.getRawParameterValue("lpfSlope")->load());
        parameters.lpfBypass = apvts.getRawParameterValue("lpfBypass")->load();
        parameters.band1Freq = apvts.getRawParameterValue("band1Freq")->load();
        parameters.band1Gain = apvts.getRawParameterValue("band1Gain")->load();
        parameters.band1Q = apvts.getRawParameterValue("band1Q")->load();
        parameters.band2Freq = apvts.getRawParameterValue("band2Freq")->load();
        parameters.band2Gain = apvts.getRawParameterValue("band2Gain")->load();
        parameters.band2Q = apvts.getRawParameterValue("band2Q")->load();
        parameters.band3Freq = apvts.getRawParameterValue("band3Freq")->load();
        parameters.band3Gain = apvts.getRawParameterValue("band3Gain")->load();
        parameters.band3Q = apvts.getRawParameterValue("band3Q")->load();
        parameters.band4Freq = apvts.getRawParameterValue("band4Freq")->load();
        parameters.band4Gain = apvts.getRawParameterValue("band4Gain")->load();
        parameters.band4Q = apvts.getRawParameterValue("band4Q")->load();
        parameters.band1Bell = apvts.getRawParameterValue("band1Bell")->load();
        parameters.band4Bell = apvts.getRawParameterValue("band4Bell")->load();
    }

    void prepare(double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate;
        bufferSize = maxBlockSize;
        dsp::ProcessSpec spec;
        spec.sampleRate = newSampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numOutputs;
        processChain.prepare(spec);
    }

    void process(AudioBuffer<float>& inputBuffer)
    {
        setupHPF();
        setupLPF();
        setupBands();
        dsp::AudioBlock<float> block(inputBuffer);
        dsp::ProcessContextReplacing<float> context(block);
        processChain.process(context);
    }

private:
    void setupHPF()
    {
        if (parameters.hpfBypass)
        {
            processChain.setBypassed<ChainIndex::HPF>(true);
        }
        else
        {
            processChain.setBypassed<ChainIndex::HPF>(false);
            *processChain.get<ChainIndex::HPF>().state = 
                *dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(
                    parameters.hpfFreq, sampleRate, 2 * (parameters.hpfSlope + 1))[0];
        }
    }

    void setupLPF()
    {
        if (parameters.lpfBypass)
        {
            processChain.setBypassed<ChainIndex::LPF>(true);
        }
        else
        {
            processChain.setBypassed<ChainIndex::LPF>(false);
            *processChain.get<ChainIndex::LPF>().state = 
                *dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(
                    parameters.lpfFreq, sampleRate, 2 * (parameters.lpfSlope + 1))[0];
        }
    }

    void setupBands()
    {
        *processChain.get<ChainIndex::Band1>().state = (parameters.band1Bell) ?
            *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, parameters.band1Freq,
                parameters.band1Q, Decibels::decibelsToGain(parameters.band1Gain)) :
            *dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, parameters.band1Freq,
                parameters.band1Q, Decibels::decibelsToGain(parameters.band1Gain));
        *processChain.get<ChainIndex::Band2>().state = 
            *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, parameters.band2Freq, 
                parameters.band2Q, Decibels::decibelsToGain(parameters.band2Gain));
        *processChain.get<ChainIndex::Band3>().state = 
            *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, parameters.band3Freq, 
                parameters.band3Q, Decibels::decibelsToGain(parameters.band3Gain));
        *processChain.get<ChainIndex::Band4>().state = (parameters.band4Bell) ?
            *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, parameters.band4Freq,
                parameters.band4Q, Decibels::decibelsToGain(parameters.band4Gain)) :
            *dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, parameters.band4Freq,
                parameters.band4Q, Decibels::decibelsToGain(parameters.band4Gain));
    }

    double sampleRate{ 0.0 };
    int bufferSize{ 0 };
    Parameters parameters;
    using StereoFilter = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, 
        dsp::IIR::Coefficients<float>>;
    dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter, 
        StereoFilter, StereoFilter> processChain;
    enum ChainIndex{ HPF, LPF, Band1, Band2, Band3, Band4 };
};