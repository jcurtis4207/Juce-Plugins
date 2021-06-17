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

// struct for all the parameter values
struct ChainSettings 
{
    bool band1Bell{ false }, band4Bell{ false },
        hpfBypass{ false }, lpfBypass{ false };
    float hpfFreq{ 0 }, lpfFreq{ 0 },
        band1Freq{ 0 }, band1Gain{ 0 },
        band2Freq{ 0 }, band2Gain{ 0 }, band2Q{ 0 },
        band3Freq{ 0 }, band3Gain{ 0 }, band3Q{ 0 },
        band4Freq{ 0 }, band4Gain{ 0 };
    int hpfSlope{ 0 }, lpfSlope{ 0 };
};
// helper to get all parameter values
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& parameters);

class EeqAudioProcessor : public juce::AudioProcessor
{
public:
    EeqAudioProcessor();
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    ~EeqAudioProcessor() override {}
    const juce::String getName() const override { return JucePlugin_Name; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}
    void releaseResources() override {}
    bool hasEditor() const override { return true; }

    // object for parameters
    juce::AudioProcessorValueTreeState parameters;
    // array for filter slopes
    juce::StringArray filterSlopes{ "12 dB/Oct", "24 dB/Oct", "36 dB/Oct" };

private:
    // setup namespaces
    using EqBand = juce::dsp::IIR::Filter<float>;
    using MonoChain = juce::dsp::ProcessorChain<EqBand, EqBand, EqBand, EqBand, EqBand, EqBand>;
    // create processor chain objects
    MonoChain leftChain, rightChain;
    // used to access chain elements by their index
    enum ChainIndex
    {
        HPF, LPF, Band1, Band2, Band3, Band4
    };
    // array of band default frequencies
    const float defaultFreq[4] { 60.0f, 400.0f, 2000.0f, 8000.0f };
    // functions to set processor chain coefficients from parameters
    void updateEqBands(const ChainSettings& chainSettings);
    void updateFilters(const ChainSettings& chainSettings);
    void setCoefficients(juce::AudioProcessorValueTreeState& apvts);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessor)
};
