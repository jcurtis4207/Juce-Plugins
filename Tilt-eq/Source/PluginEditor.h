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
#include "../../Modules/GUI-Components.h"

class TilteqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TilteqAudioProcessorEditor(TilteqAudioProcessor&);
    ~TilteqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    TilteqAudioProcessor& audioProcessor;

    BgImage bgImage;
    PowerLine powerLine{ "Tilt-eq", "Jacob Curtis", 30 };
    SmallKnob freqKnob{ "Freq", "Hz" }, tiltKnob{ "Tilt", "dB" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach, gainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TilteqAudioProcessorEditor)
};
