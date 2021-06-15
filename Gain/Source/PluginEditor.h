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
#include "Meter.h"
#include "GainLookAndFeel.h"

class GainAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    GainAudioProcessorEditor(GainAudioProcessor&);
    ~GainAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override {}

    // listener for slider
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // reference to processor block
    GainAudioProcessor& audioProcessor;
    // create meter object
    Meter meter;
    // define components
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::TextButton phaseButton;
    juce::Label phaseLabel;
    // define parameter attachments
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttach;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttach;
    // define component position and size
    const int gainXPosition{ 20 };
    const int gainYPosition{ 80 };
    const int gainSliderWidth{ 80 };
    const int phaseYPosition{ 200 };
    const int phaseButtonWidth{ 30 };
    const int phaseXPosition{ (gainSliderWidth / 2) + gainXPosition - (phaseButtonWidth / 2) };
    const int labelHeight{ 20 };
    const juce::Rectangle<int> gainSliderBounds{ gainXPosition, gainYPosition, gainSliderWidth, gainSliderWidth };
    const juce::Rectangle<int> gainLabelBounds{ gainXPosition, gainYPosition - labelHeight, gainSliderWidth, labelHeight };
    const juce::Rectangle<int> phaseButtonBounds{ phaseXPosition, phaseYPosition, phaseButtonWidth, phaseButtonWidth };
    const juce::Rectangle<int> phaseLabelBounds{ phaseXPosition - 15, phaseYPosition - labelHeight, 60, labelHeight };
    const juce::Rectangle<int> meterBounds{ 120, 40, meter.getMeterWidth(), meter.getMeterHeight() };
    // create look and feel object
    GainLookAndFeel gainLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainAudioProcessorEditor)
};
