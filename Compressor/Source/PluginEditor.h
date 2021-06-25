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
#include "GUI/Meter.h"
#include "GUI/CompressorLookAndFeel.h"
#include "GUI/PowerLine.h"

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
    juce::Slider thresholdSlider, attackSlider, releaseSlider, ratioSlider, makeUpSlider, scFreqSlider, mixSlider;
    juce::TextButton scBypassButton{ "Bypass" }, stereoButton{ "Stereo" };
    juce::Label thresholdLabel, attackLabel, releaseLabel, ratioLabel, makeUpLabel, scFreqLabel, scBypassLabel, grLabel, stereoLabel, mixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, attackAttach, releaseAttach, ratioAttach, makeUpAttach, scFreqAttach, mixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scBypassAttach, stereoAttach;
    // create gr meter
    Meter meter;
    // create look and feel object
    CompressorLookAndFeel compressorLookAndFeel;
    // create powerline object
    PowerLine powerLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorAudioProcessorEditor)
};
