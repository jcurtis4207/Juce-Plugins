/*
*   Limiter Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>

class LimiterAudioProcessor : public juce::AudioProcessor
{
public:
    LimiterAudioProcessor();
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

    ~LimiterAudioProcessor() override {}
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
    // variables for meter
    float bufferMagnitudeIn{ 0.0f };
    float bufferMagnitudeOut{ 0.0f };

private:
    // variables for limiter
    float threshold{ 0.0f };
    float release{ 1.0f };
    float ceiling{ 0.0f };
    // create processor chain objects with a limiter and output gain
    juce::dsp::ProcessorChain<juce::dsp::Limiter<float>, juce::dsp::Gain<float>> leftChain, rightChain;
    enum ChainIndex
    {
        Limiter, Gain
    };
    // function to update limiter from parameters
    void updateLimiterValues(juce::AudioProcessorValueTreeState& apvts);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterAudioProcessor)
};
