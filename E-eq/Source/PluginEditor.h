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
    std::array<OuterKnob, numFilters> slopeKnobs;
    std::array<SmallKnob, numFilters> filterKnobs{ SmallKnob("", "Hz"), SmallKnob("", "Hz") };
    std::array<juce::String, numBands> filterParamIDs{ "hpfFreq", "lpfFreq", "hpfSlope", "lpfSlope" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterKnobsAttach[4];
    // parametric bands
    std::array<OuterKnob, numBands> freqKnobs{ "Hz", "Hz", "Hz", "Hz" };
    std::array<SmallKnob, numBands> gainKnobs{ SmallKnob("", "dB"), SmallKnob("", "dB"), SmallKnob("", "dB"), SmallKnob("", "dB") };
    std::array<SmallKnob, numBands> qKnobs{ SmallKnob("", "Q"), SmallKnob("", "Q"), SmallKnob("", "Q"), SmallKnob("", "Q") };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandKnobsAttach[12];
    // buttons
    SmallButton hpfBypassButton{ "Bypass" }, lpfBypassButton{ "Bypass" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfBypassAttach, lpfBypassAttach;
    SmallButton band1BellButton{ "Bell" }, band4BellButton{ "Bell" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> band1BellAttach, band4BellAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessorEditor)
};