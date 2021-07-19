/*
*   MultiBandComp Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "MultiBandComp.h"

class MultiBandCompAudioProcessor : public juce::AudioProcessor
{
public:
    MultiBandCompAudioProcessor();
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

    ~MultiBandCompAudioProcessor() override {}
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
    float gainReduction[8];
    bool listen[4]{ false, false, false, false };

private:
    MultiBandComp multibandComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandCompAudioProcessor)
};