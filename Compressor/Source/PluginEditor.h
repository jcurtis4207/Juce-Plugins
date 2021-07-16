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
#include "../../Modules/GUI-Components.h"
#include "../../Modules/Meters.h"

class CompressorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CompressorAudioProcessorEditor(CompressorAudioProcessor&);
    ~CompressorAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    CompressorAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Compressor", "Jacob Curtis", 30 };
    GainReductionMeter grMeter;
    SmallKnob thresholdKnob{ "Threshold", "dB"}, attackKnob{ "Attack", "ms" }, releaseKnob{ "Release", "ms" },
        ratioKnob{ "Ratio", ": 1" }, makeUpKnob{ "Make Up", "dB" }, scFreqKnob{ "SC Freq", "Hz" }, mixKnob{ "Mix", "%" };
    SmallButton scBypassButton{ "SC Bypass" }, stereoButton{ "Stereo" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, attackAttach, releaseAttach, ratioAttach, makeUpAttach, scFreqAttach, mixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scBypassAttach, stereoAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorAudioProcessorEditor)
};
