/*
*   E-eq Plugin
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

class EeqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EeqAudioProcessorEditor(EeqAudioProcessor&);
    ~EeqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    EeqAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "E - eq", "Jacob Curtis", 30 };
    MultiLabel multiLabel{ "Q" };
    // filters
    OuterKnob slopeKnobs[2];
    SmallKnob filterKnobs[2];
    juce::String filterParamIDs[4]{ "hpfFreq", "lpfFreq", "hpfSlope", "lpfSlope" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterKnobsAttach[4];
    // parametric bands
    OuterKnob freqKnobs[4];
    SmallKnob gainKnobs[4];
    SmallKnob qKnobs[4];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandKnobsAttach[12];
    // buttons
    SmallButton hpfBypassButton{ "Bypass" }, lpfBypassButton{ "Bypass" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfBypassAttach, lpfBypassAttach;
    SmallButton band1BellButton{ "Bell" }, band4BellButton{ "Bell" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> band1BellAttach, band4BellAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessorEditor)
};