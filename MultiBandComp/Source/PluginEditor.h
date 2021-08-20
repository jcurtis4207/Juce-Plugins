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
    std::array<MultiLabel, numBands> bandLabels{ "Band 1", "Band 2", "Band 3", "Band 4" };
    std::array<GainReductionMeter, numBands> grMeters;
    std::array<SmallKnob, numBands - 1> freqKnobs{ SmallKnob("Freq", "Hz"), 
        SmallKnob("Freq", "Hz"), SmallKnob("Freq", "Hz") };
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numBands - 1> freqAttach;
    std::array<OuterKnob, numBands> ratioKnobs{ ": 1", ": 1" , ": 1" , ": 1" };
    std::array<SmallKnob, numBands> thresholdKnobs{ SmallKnob("", "dB"), SmallKnob("", "dB"), 
        SmallKnob("", "dB"), SmallKnob("", "dB") };
    std::array<SmallKnob, numBands> attackKnobs{ SmallKnob("Attack", "ms"), SmallKnob("Attack", "ms"), 
        SmallKnob("Attack", "ms"), SmallKnob("Attack", "ms") };
    std::array<SmallKnob, numBands> releaseKnobs{ SmallKnob("Release", "ms"), SmallKnob("Release", "ms"), 
        SmallKnob("Release", "ms"), SmallKnob("Release", "ms") };
    std::array<SmallKnob, numBands> makeUpKnobs{ SmallKnob("Gain", "dB"), SmallKnob("Gain", "dB"), 
        SmallKnob("Gain", "dB"), SmallKnob("Gain", "dB") };
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, numBands> thresholdAttach, 
        ratioAttach, attackAttach, releaseAttach, makeUpAttach;
    SmallButton stereoButton{ "Stereo" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach;
    MultiLabel listenLabel{ "Listen" };
    std::array<SmallButton, numBands> listenButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiBandCompAudioProcessorEditor)
};
