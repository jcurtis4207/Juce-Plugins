/*
*   De-esser Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../Modules/GUI-Components.h"
#include "../../Modules/Meters.h"

class DeesserAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DeesserAudioProcessorEditor(DeesserAudioProcessor&);
    ~DeesserAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    DeesserAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "De-esser", "Jacob Curtis", 30 };
    GainReductionMeter grMeter;
    SmallKnob thresholdKnob{ "Threshold", "dB" }, crossoverKnob{ "Frequency", "Hz" }, 
        attackKnob{ "Attack", "ms" }, releaseKnob{ "Release", "ms" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, 
        crossoverAttach, attackAttach, releaseAttach;
    SmallButton stereoButton{ "Stereo" }, wideButton{ "Wide" }, listenButton{ "Listen" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach, wideAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeesserAudioProcessorEditor)
};
