/*
*   Tilt-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

TilteqAudioProcessorEditor::TilteqAudioProcessorEditor(TilteqAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // setup sliders
    freqKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    freqKnob.setLookAndFeel(&tiltLookAndFeel);
    freqKnob.setTextValueSuffix(" Hz");
    addAndMakeVisible(freqKnob);
    freqKnob.setBounds(30, 40, 50, 50);
    freqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "freq", freqKnob);
    gainKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainKnob.setLookAndFeel(&tiltLookAndFeel);
    gainKnob.setTextValueSuffix(" dB");
    addAndMakeVisible(gainKnob);
    gainKnob.setBounds(120, 40, 50, 50);
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainKnob);
    // setup labels
    freqLabel.setText("Freq", juce::NotificationType::dontSendNotification);
    freqLabel.setFont(freqLabel.getFont().withHeight(12));
    freqLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::grey);
    freqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(freqLabel);
    freqLabel.setBounds(freqKnob.getX(), freqKnob.getBottom(), 50, 20);
    gainLabel.setText("Tilt", juce::NotificationType::dontSendNotification);
    gainLabel.setFont(gainLabel.getFont().withHeight(12));
    gainLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::grey);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    gainLabel.setBounds(gainKnob.getX(), gainKnob.getBottom(), 50, 20);

    setSize(200, 150);
}

TilteqAudioProcessorEditor::~TilteqAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void TilteqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw tilt diagrams around gain label
    float yPosition = gainLabel.getY() + gainLabel.getHeight() / 2;
    float leftXPosition = gainLabel.getX() + 2;
    float rightXPosition = gainLabel.getRight() - 2;
    g.setColour(juce::Colours::white);
    g.drawLine(juce::Line<float>(leftXPosition - 20, yPosition, leftXPosition, yPosition), 1.0f);
    g.drawLine(juce::Line<float>(rightXPosition, yPosition, rightXPosition + 20, yPosition), 1.0f);
    g.setColour(juce::Colour(0xff1a2e78));  // blue
    g.drawLine(juce::Line<float>(leftXPosition - 19, yPosition - 4, leftXPosition - 1, yPosition + 4).toFloat(), 2.0f);
    g.setColour(juce::Colour(0xff7c1e1e));  // red
    g.drawLine(juce::Line<float>(rightXPosition + 1, yPosition + 4, rightXPosition + 19, yPosition - 4), 2.0f);
}

void TilteqAudioProcessorEditor::resized()
{
}
