/*
*   Compressor Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Compressored on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "Compressor.h"

class CompressorAudioProcessor : public juce::AudioProcessor
{
public:
    CompressorAudioProcessor();
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
    void releaseResources() override;

    ~CompressorAudioProcessor() override {}
    const juce::String getName() const override { return JucePlugin_Name; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}
    bool hasEditor() const override { return true; }

    // object for parameters
    juce::AudioProcessorValueTreeState parameters;
    // value for gain reduction meter
    float gainReductionValue{ 0.0f };

private:
    // create compressor
    Compressor compressor;
    // function to update compressor values
    void CompressorAudioProcessor::updateCompressorValues(juce::AudioProcessorValueTreeState& apvts);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorAudioProcessor)
};
