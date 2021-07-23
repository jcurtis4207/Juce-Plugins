/*
*   Reverb Plugin
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

class ReverbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ReverbAudioProcessorEditor(ReverbAudioProcessor&);
    ~ReverbAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    ReverbAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Reverb", "Jacob Curtis", 30 };
    BigKnob sizeKnob{ "Size" };
    SmallKnob dampingKnob{ "Damping" },
        mixKnob{ "Mix", "%" },
        predelayKnob{ "Predelay", "ms" },
        depthKnob{ "Depth" },
        rateKnob{ "Rate", "Hz" },
        hpfKnob{ "HPF", "Hz" },
        lpfKnob{ "LPF", "Hz" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttach, dampingAttach, 
        mixAttach, predelayAttach, depthAttach, rateAttach, hpfAttach, lpfAttach;
    MultiLabel filterLabel{ "Filters" }, modLabel{ "Mod" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbAudioProcessorEditor)
};
