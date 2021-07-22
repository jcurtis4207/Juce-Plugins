/*
*   Delay Plugin
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

class DelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DelayAudioProcessorEditor(DelayAudioProcessor&);
    ~DelayAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    DelayAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Delay", "Jacob Curtis", 30 };
    MultiLabel modLabel{ "Mod" }, filterLabel{ "Filters" };
    BigKnob delayKnob{ "Delay Time", "ms" };
    BigKnob subdivisionKnob{ "Delay Subdivision" };
    SmallKnob feedbackKnob{ "FB" },
        mixKnob{ "Mix", "%" },
        widthKnob{ "Width", "ms" },
        driveKnob{ "Drive" },
        hpfKnob{ "HPF", "Hz" },
        lpfKnob{ "LPF", "Hz" },
        depthKnob{ "Depth" },
        rateKnob{ "Rate", "Hz" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttach, subdivisionAttach,
        feedbackAttach, mixAttach, widthAttach, driveAttach, hpfAttach, lpfAttach, depthAttach, rateAttach;
    SmallButton syncButton{ "Sync" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttach;
    void switchKnob(bool isSync);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessorEditor)
};
