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
#include "Equalizer.h"

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
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void releaseResources() override {}
    bool hasEditor() const override { return true; }

    juce::AudioProcessorValueTreeState parameters;
    juce::StringArray filterSlopes{ "12 dB/Oct", "24 dB/Oct", "36 dB/Oct" };

private:
    Equalizer equalizer;
    const float defaultFreq[4] { 60.0f, 400.0f, 2000.0f, 8000.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessor)
};
