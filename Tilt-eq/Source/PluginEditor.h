/*
*   Tilt-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/PowerLine.h"
#include "GUI/TiltLookAndFeel.h"

class TilteqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TilteqAudioProcessorEditor(TilteqAudioProcessor&);
    ~TilteqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TilteqAudioProcessor& audioProcessor;
    // create gui components
    juce::Slider freqKnob, gainKnob;
    juce::Label freqLabel, gainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach, gainAttach;
    // create look and feel object
    TiltLookAndFeel tiltLookAndFeel;
    // create powerline object
    PowerLine powerLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TilteqAudioProcessorEditor)
};
