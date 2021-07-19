/*
*   MultiBandComp Plugin
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

class MultiBandCompAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MultiBandCompAudioProcessorEditor(MultiBandCompAudioProcessor&);
    ~MultiBandCompAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    MultiBandCompAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Multiband Comp", "Jacob Curtis", 30 };
    MultiLabel bandLabels[4]{ "Band 1", "Band 2", "Band 3", "Band 4" };
    GainReductionMeter grMeters[4];
    SmallKnob freqKnobs[3]{ SmallKnob("Freq", "Hz"), SmallKnob("Freq", "Hz"), SmallKnob("Freq", "Hz") };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach[3];
    OuterKnob ratioKnobs[4]{ ": 1", ": 1" , ": 1" , ": 1" };
    SmallKnob thresholdKnobs[4]{ SmallKnob("", "dB"), SmallKnob("", "dB"), SmallKnob("", "dB"), SmallKnob("", "dB") };
    SmallKnob attackKnobs[4]{ SmallKnob("Attack", "ms"), SmallKnob("Attack", "ms"), SmallKnob("Attack", "ms"), SmallKnob("Attack", "ms") };
    SmallKnob releaseKnobs[4]{ SmallKnob("Release", "ms"), SmallKnob("Release", "ms"), SmallKnob("Release", "ms"), SmallKnob("Release", "ms") };
    SmallKnob makeUpKnobs[4]{ SmallKnob("Gain", "dB"), SmallKnob("Gain", "dB"), SmallKnob("Gain", "dB"), SmallKnob("Gain", "dB") };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach[4], ratioAttach[4], attackAttach[4], releaseAttach[4], makeUpAttach[4];
    SmallButton stereoButton{ "Stereo" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandCompAudioProcessorEditor)
};
