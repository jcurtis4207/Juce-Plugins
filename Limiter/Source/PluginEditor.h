/*
*   Limiter Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LimiterLookAndFeel.h"
#include "Meter.h"

class LimiterAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    LimiterAudioProcessorEditor(LimiterAudioProcessor&);
    ~LimiterAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LimiterAudioProcessor& audioProcessor;
    // gui components
    juce::Slider thresholdSlider, ceilingSlider, releaseSlider, linkKnob;
    juce::Label thresholdLabel, ceilingLabel, releaseLabel, linkLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttach, ceilingAttach, releaseAttach;
    // linked parameter values
    float thresholdValue{ 0.0f };
    float ceilingValue{ 0.0f };
    // flag for modifying parameters
    bool linkFlag{ false };
    // create look and feel
    LimiterLookAndFeel limiterLookAndFeel;
    // create meter object
    Meter meter;
    // function for trim knob to modify parameters
    void sliderValueChanged(juce::Slider* slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LimiterAudioProcessorEditor)
};
