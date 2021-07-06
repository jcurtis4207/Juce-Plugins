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
#include "GUI/DistortionLookAndFeel.h"
#include "GUI/PowerLine.h"

class DistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor(DistortionAudioProcessor&);
    ~DistortionAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DistortionAudioProcessor& audioProcessor;
    DistortionLookAndFeel distortionLookAndFeel;
    PowerLine powerLine;
    // gui components
    juce::Slider driveKnob, volumeKnob, mixKnob, angerKnob, hpfKnob, lpfKnob, shapeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttach, volumeAttach, mixAttach, angerAttach, hpfAttach, lpfAttach, shapeAttach;
    juce::TextButton shapeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> shapeButtonAttach;
    juce::TextButton typeButtons[4];
    void buttonClicked(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionAudioProcessorEditor)
};
