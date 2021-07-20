/*
*   Gate Plugin
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

class GateAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GateAudioProcessorEditor(GateAudioProcessor&);
    ~GateAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    GateAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Gate", "Jacob Curtis", 30 };
    GainReductionMeter grMeter;
    SmallKnob thresholdKnob{ "Threshold", "dB" },
        ratioKnob{ "Ratio", ": 1" },
        attackKnob{ "Attack", "ms" },
        releaseKnob{ "Release", "ms" },
        holdKnob{ "Hold", "ms" },
        hpfKnob{ "HPF", "Hz" },
        lpfKnob{ "LPF", "Hz" };
    SmallButton scEnableButton{ "SC Enable" }, listenButton{ "Listen" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, ratioAttach, attackAttach, releaseAttach, holdAttach, hpfAttach, lpfAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scEnableAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GateAudioProcessorEditor)
};
