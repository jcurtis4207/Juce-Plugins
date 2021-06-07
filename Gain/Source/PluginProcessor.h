/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define GAIN_ID "gain"
#define GAIN_NAME "Gain"
#define GAIN_RANGE_LOW -30.0f
#define GAIN_RANGE_HIGH 30.0f
#define GAIN_RANGE_INTERVAL 0.5f
#define PHASE_ID "phase"
#define PHASE_NAME "Phase"

//==============================================================================
class GainAudioProcessor : public juce::AudioProcessor
{
public:
    GainAudioProcessor();
    ~GainAudioProcessor() override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //====================================================================
    // variable for slider gain
    float gain;
    // variable for gain before GUI is opened
    float previousGain;
    // variable for phase inversion
    float phase;
    // object to hold plugin parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainAudioProcessor)
};
