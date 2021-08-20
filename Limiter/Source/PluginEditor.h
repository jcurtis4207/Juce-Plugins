/*
*   Limiter Plugin
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

class LimiterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    LimiterAudioProcessorEditor(LimiterAudioProcessor&);
    ~LimiterAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    LimiterAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Limiter", "Jacob Curtis", 30 };
    GainReductionMeter grMeter;
    VerticalSlider thresholdSlider{ "dB" }, ceilingSlider{ "dB" }, releaseSlider{ "ms" };
    GreyLabel thresholdLabel{ "Threshold" }, ceilingLabel{ "Ceiling" }, releaseLabel{ "Release" };
    LinkKnob linkKnob;
    SmallButton stereoButton{ "Stereo" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, 
        ceilingAttach, releaseAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach;
    // link operation
    float thresholdValue{ 0.0f };
    float ceilingValue{ 0.0f };
    bool linkFlag{ false };
    void linkValueChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterAudioProcessorEditor)
};
