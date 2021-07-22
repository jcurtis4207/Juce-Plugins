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
    BigKnob delayKnob{ "Delay Time", "ms" };
    SmallKnob feedbackKnob{ "FB" },
        mixKnob{ "Mix", "%" },
        widthKnob{ "Width", "ms" },
        driveKnob{ "Drive" },
        hpfKnob{ "HPF", "Hz" },
        lpfKnob{ "LPF", "Hz" },
        depthKnob{ "Depth" },
        rateKnob{ "Rate", "Hz" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttach,
        feedbackAttach, mixAttach, widthAttach, driveAttach, hpfAttach, lpfAttach, depthAttach, rateAttach;
    MultiLabel modLabel{ "Mod" }, filterLabel{ "Filters" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessorEditor)
};
