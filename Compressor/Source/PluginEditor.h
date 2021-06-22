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
#include "PluginProcessor.h"
#include "Meter.h"
#include "CompressorLookAndFeel.h"
#include "PowerLine.h"

class CompressorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CompressorAudioProcessorEditor(CompressorAudioProcessor&);
    ~CompressorAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CompressorAudioProcessor& audioProcessor;
    // define gui components
    juce::Slider thresholdSlider, attackSlider, releaseSlider, ratioSlider, makeUpSlider, scFreqSlider;
    juce::TextButton scBypassButton{ "Bypass" };
    juce::Label thresholdLabel, attackLabel, releaseLabel, ratioLabel, makeUpLabel, scFreqLabel, scBypassLabel, grLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, attackAttach, releaseAttach, ratioAttach, makeUpAttach, scFreqAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scBypassAttach;
    // create gr meter
    Meter meter;
    // create look and feel object
    CompressorLookAndFeel compressorLookAndFeel;
    // create powerline object
    PowerLine powerLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorAudioProcessorEditor)
};
