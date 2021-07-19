/*
*   Gain Plugin
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

class GainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GainAudioProcessorEditor(GainAudioProcessor&);
    ~GainAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    GainAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Gain", "Jacob Curtis", 30 };
    SmallKnob gainKnob{ "Gain", "dB"};
    SmallButton phaseButton{ "Phase" };
    Meter meter;

    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> gainAttach;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> phaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainAudioProcessorEditor)
};
