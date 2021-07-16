/*
*   Clipper Plugin
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

class ClipperAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ClipperAudioProcessorEditor(ClipperAudioProcessor&);
    ~ClipperAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    ClipperAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Clipper", "Jacob Curtis", 30 };
    VerticalSlider thresholdSlider{ "dB" }, ceilingSlider{ "dB" };
    GreyLabel thresholdLabel{ "Threshold" }, ceilingLabel{ "Ceiling" };
    LinkKnob linkKnob;
    GainReductionMeter grMeter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, ceilingAttach;

    // link operation
    float thresholdValue{ 0.0f };
    float ceilingValue{ 0.0f };
    bool linkFlag{ false };
    void linkValueChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipperAudioProcessorEditor)
};
