/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAudioProcessorEditor::GainAudioProcessorEditor(GainAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // setup slider
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);   // set slider to rotary
    gainSlider.setRange(GAIN_RANGE_LOW, GAIN_RANGE_HIGH, 0.01f);                // set range, and increment is overridden by parameter
    gainSlider.setBounds(80, 80, 80, 80);                                       // set bounds and size
    gainSlider.setLookAndFeel(&gainLookAndFeel);                                // set custom appearance
    gainSlider.addListener(this);                                               // apply listener to slider
    addAndMakeVisible(gainSlider);
    // attach slider to parameter via GAIN_ID
    sliderAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, GAIN_ID, gainSlider));
    // setup gain label
    gainLabel.setText("Gain", juce::NotificationType::dontSendNotification);    // set label text
    gainLabel.setJustificationType(juce::Justification::centred);               // set center justification
    gainLabel.setBounds(80, 60, 80, 20);                                        // set bounds and size
    addAndMakeVisible(gainLabel);
    // setup button
    phaseButton.setButtonText(juce::CharPointer_UTF8("\xc3\x98"));  // set button text - unicode U+00D8 -> ascii -> hex = \xc3\x98
    phaseButton.setBounds(35, 90, 30, 30);                          // set bounds and size of button
    phaseButton.setLookAndFeel(&gainLookAndFeel);                   // set custom appearance
    phaseButton.addListener(this);                                  // apply listener to button
    addAndMakeVisible(phaseButton);
    // attach button to parameter via PHASE_ID
    buttonAttach.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(audioProcessor.parameters, PHASE_ID, phaseButton));
    // setup phase label
    phaseLabel.setText("Phase", juce::NotificationType::dontSendNotification);  // set label text
    phaseLabel.setJustificationType(juce::Justification::centred);              // set center justification
    phaseLabel.setBounds(20, 60, 60, 20);                                       // set bounds and size
    addAndMakeVisible(phaseLabel);

    // set default plugin window size
    setSize(200, 300);
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
    // bad things happen if look and feel isn't reset here
    setLookAndFeel(nullptr);
}

void GainAudioProcessorEditor::paint(juce::Graphics& g)
{
    // fill background with dark grey (argb)
    g.fillAll(juce::Colour(0xff121212));
}

void GainAudioProcessorEditor::resized()
{
}

// listener override for slider
void GainAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // if listener is triggered by gain slider
    if (slider == &gainSlider)
    {
        // tell the processor to set the gain according to the slider value
        audioProcessor.gain = (float)gainSlider.getValue();
    }
}

// listener override for button
void GainAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    // if listener is triggered by phase button
    if (button == &phaseButton)
    {
        // switch phase variable between 0 and 1
        audioProcessor.phase = (audioProcessor.phase == 0.0f) ? 1.0f : 0.0f;
    }
}