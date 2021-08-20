/*
*   Distortion Plugin
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

class DistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor(DistortionAudioProcessor&);
    ~DistortionAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    DistortionAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Distortion", "Jacob Curtis", 30 };
    BigKnob driveKnob{ "Drive" };
    SmallKnob volumeKnob{ "Volume", "dB" }, mixKnob{ "Mix", "%" }, angerKnob{ "Anger" }, 
        offsetKnob{ "Offset" }, hpfKnob{"HPF", "Hz"}, lpfKnob{"LPF", "Hz"}, shapeKnob{"Shape", "dB"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttach, 
        volumeAttach, mixAttach, angerAttach, offsetAttach, hpfAttach, lpfAttach, shapeAttach;
    SmallButton shapeButton{ "Tilt" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> shapeButtonAttach;
    std::array<BigButton, 4> typeButtons;
    MultiLabel multiLabel{ "Drive Type" };

    void buttonClicked(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionAudioProcessorEditor)
};