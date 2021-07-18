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

// struct to hold parameter values
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
    ParameterValues getParameterValues(juce::AudioProcessorValueTreeState& apvts)
    {
        ParameterValues newValues;
        // get all paramter values for filters
        newValues.hpfFreq = apvts.getRawParameterValue("hpfFreq")->load();
        newValues.hpfSlope = (int)apvts.getRawParameterValue("hpfSlope")->load();
        newValues.hpfBypass = apvts.getRawParameterValue("hpfBypass")->load();
        newValues.lpfFreq = apvts.getRawParameterValue("lpfFreq")->load();
        newValues.lpfSlope = (int)apvts.getRawParameterValue("lpfSlope")->load();
        newValues.lpfBypass = apvts.getRawParameterValue("lpfBypass")->load();
        // get all parameter values for parametric bands
        newValues.band1Freq = apvts.getRawParameterValue("band1Freq")->load();
        newValues.band1Gain = apvts.getRawParameterValue("band1Gain")->load();
        newValues.band1Q = apvts.getRawParameterValue("band1Q")->load();
        newValues.band2Freq = apvts.getRawParameterValue("band2Freq")->load();
        newValues.band2Gain = apvts.getRawParameterValue("band2Gain")->load();
        newValues.band2Q = apvts.getRawParameterValue("band2Q")->load();
        newValues.band3Freq = apvts.getRawParameterValue("band3Freq")->load();
        newValues.band3Gain = apvts.getRawParameterValue("band3Gain")->load();
        newValues.band3Q = apvts.getRawParameterValue("band3Q")->load();
        newValues.band4Freq = apvts.getRawParameterValue("band4Freq")->load();
        newValues.band4Gain = apvts.getRawParameterValue("band4Gain")->load();
        newValues.band4Q = apvts.getRawParameterValue("band4Q")->load();
        newValues.band1Bell = apvts.getRawParameterValue("band1Bell")->load();
        newValues.band4Bell = apvts.getRawParameterValue("band4Bell")->load();
        return newValues;
    }

    void prepare(const double newSampleRate, const int numChannels, const int maxBlockSize)
    {
        sampleRate = newSampleRate;
        eqBuffer.setSize(numChannels, maxBlockSize);
        // initialize dsp
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = newSampleRate;
        spec.maximumBlockSize = maxBlockSize;
        spec.numChannels = numChannels;
        // prepare the process chain
        processChain.prepare(spec);
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        const auto& outputBuffer = context.getOutputBlock();
        const int bufferSize = static_cast<int>(outputBuffer.getNumSamples());
        // copy input to eq buffer
        juce::FloatVectorOperations::copy(eqBuffer.getWritePointer(0), outputBuffer.getChannelPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(eqBuffer.getWritePointer(1), outputBuffer.getChannelPointer(1), bufferSize);
        // apply filter processing to buffer
        juce::dsp::AudioBlock<float> filterBlock(eqBuffer);
        juce::dsp::ProcessContextReplacing<float> filterContext(filterBlock);
        processChain.process(filterContext);
        // copy eq buffer to output
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(0), eqBuffer.getReadPointer(0), bufferSize);
        juce::FloatVectorOperations::copy(outputBuffer.getChannelPointer(1), eqBuffer.getReadPointer(1), bufferSize);
    }

    // get parameter values and setup filter coefficients
    void setCoefficients(juce::AudioProcessorValueTreeState& apvts)
    {
        // get parameters structure
        auto parameterValues = getParameterValues(apvts);
        // set coefficients from parameters
        updateFilters(parameterValues);
        updateEqBands(parameterValues);
    }

    // update filter coefficients from settings struct
    void updateFilters(const ParameterValues& inputValues)
    {
        // bypass hpf
        if (inputValues.hpfBypass)
        {
            processChain.setBypassed<ChainIndex::HPF>(true);
        }
        // update hpf coefficients
        else
        {
            processChain.setBypassed<ChainIndex::HPF>(false);
            auto hpfCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(inputValues.hpfFreq, sampleRate, 2 * (inputValues.hpfSlope + 1));
            *processChain.get<ChainIndex::HPF>().state = *hpfCoefficients[0];
        }
        // bypass lpf
        if (inputValues.lpfBypass)
        {
            processChain.setBypassed<ChainIndex::LPF>(true);
        }
        // update lpf coefficients
        else
        {
            processChain.setBypassed<ChainIndex::LPF>(false);
            auto lpfCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(inputValues.lpfFreq, sampleRate, 2 * (inputValues.lpfSlope + 1));
            *processChain.get<ChainIndex::LPF>().state = *lpfCoefficients[0];
        }
    }

    // update parametric bands coefficients from settings struct
    void updateEqBands(const ParameterValues& inputValues)
    {
        // create peak/shelf coefficients for all bands
        auto band1Coefficients = (inputValues.band1Bell) ?
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band1Freq, inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain)) :
            juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, inputValues.band1Freq, inputValues.band1Q, juce::Decibels::decibelsToGain(inputValues.band1Gain));
        auto band2Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band2Freq, inputValues.band2Q, juce::Decibels::decibelsToGain(inputValues.band2Gain));
        auto band3Coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band3Freq, inputValues.band3Q, juce::Decibels::decibelsToGain(inputValues.band3Gain));
        auto band4Coefficients = (inputValues.band4Bell) ?
            juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, inputValues.band4Freq, inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain)) :
            juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, inputValues.band4Freq, inputValues.band4Q, juce::Decibels::decibelsToGain(inputValues.band4Gain));
        // apply coefficients to process chain
        *processChain.get<ChainIndex::Band1>().state = *band1Coefficients;
        *processChain.get<ChainIndex::Band2>().state = *band2Coefficients;
        *processChain.get<ChainIndex::Band3>().state = *band3Coefficients;
        *processChain.get<ChainIndex::Band4>().state = *band4Coefficients;
    }

private:
    double sampleRate;
    juce::AudioBuffer<float> eqBuffer;
    using StereoFilter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    juce::dsp::ProcessorChain<StereoFilter, StereoFilter, StereoFilter, StereoFilter, StereoFilter, StereoFilter> processChain;
    enum ChainIndex
    {
        HPF, LPF, Band1, Band2, Band3, Band4
    };
};