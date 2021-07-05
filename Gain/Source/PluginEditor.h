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
#include "GUI/Meter.h"
#include "GUI/GainLookAndFeel.h"
#include "GUI/PowerLine.h"

class GainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GainAudioProcessorEditor(GainAudioProcessor&);
    ~GainAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GainAudioProcessor& audioProcessor;
    GainLookAndFeel gainLookAndFeel;
    Meter meter;
    PowerLine powerLine;
    // gui components
    juce::Slider gainKnob;
    juce::TextButton phaseButton;
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttach;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttach;
    juce::Label gainLabel{ "Gain" }, phaseLabel{ "Phase" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainAudioProcessorEditor)
};
