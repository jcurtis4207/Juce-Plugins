/*
*   De-esser Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/DeesserLookAndFeel.h"
#include "GUI/Meter.h"
#include "GUI/PowerLine.h"

class DeesserAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DeesserAudioProcessorEditor(DeesserAudioProcessor&);
    ~DeesserAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DeesserAudioProcessor& audioProcessor;
    DeesserLookAndFeel deesserLookAndFeel;
    Meter meter;
    PowerLine powerLine;
    // gui components
    juce::Slider thresholdSlider, crossoverSlider, attackSlider, releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, crossoverAttach, attackAttach, releaseAttach;
    juce::TextButton stereoButton{ "Stereo" }, wideButton{ "Wide" }, listenButton{ "Listen" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach, wideAttach, listenAttach;
    juce::Label thresholdLabel, crossoverLabel, attackLabel, releaseLabel, grLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeesserAudioProcessorEditor)
};
