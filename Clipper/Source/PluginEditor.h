/*
*   Clipper Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Meter.h"
#include "ClipperLookAndFeel.h"
#include "PowerLine.h"

class ClipperAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    ClipperAudioProcessorEditor(ClipperAudioProcessor&);
    ~ClipperAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ClipperAudioProcessor& audioProcessor;
    // gui components
    juce::Slider thresholdSlider, ceilingSlider, linkKnob;
    juce::Label thresholdLabel, ceilingLabel, linkLabel, grLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, ceilingAttach;
    // linked parameter values
    float thresholdValue{ 0.0f };
    float ceilingValue{ 0.0f };
    // flag for modifying parameters
    bool linkFlag{ false };
    // create look and feel
    ClipperLookAndFeel clipperLookAndFeel;
    // create meter objects
    Meter grMeter;
    // create powerline object
    PowerLine powerLine;
    // function for trim knob to modify parameters
    void sliderValueChanged(juce::Slider* slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipperAudioProcessorEditor)
};
