/*
*   De-esser Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

DeesserAudioProcessorEditor::DeesserAudioProcessorEditor(DeesserAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), meter(audioProcessor.gainReductionLeft, audioProcessor.gainReductionRight)
{
    // threshold
    thresholdSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    thresholdSlider.setTextValueSuffix(" dB");
    thresholdSlider.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(thresholdSlider);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold", thresholdSlider);
    thresholdLabel.setText("Threshold", juce::NotificationType::dontSendNotification);
    thresholdLabel.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(thresholdLabel);
    // crossover
    crossoverSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    crossoverSlider.setTextValueSuffix(" Hz");
    crossoverSlider.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(crossoverSlider);
    crossoverAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crossoverFreq", crossoverSlider);
    crossoverLabel.setText("Frequency", juce::NotificationType::dontSendNotification);
    crossoverLabel.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(crossoverLabel);
    // attack
    attackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    attackSlider.setTextValueSuffix(" ms");
    attackSlider.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(attackSlider);
    attackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack", attackSlider);
    attackLabel.setText("Attack", juce::NotificationType::dontSendNotification);
    attackLabel.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(attackLabel);
    // release
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    releaseSlider.setTextValueSuffix(" ms");
    releaseSlider.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(releaseSlider);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseSlider);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    releaseLabel.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(releaseLabel);
    // stereo
    stereoButton.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(stereoButton);
    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stereo", stereoButton);
    // wide
    wideButton.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(wideButton);
    wideAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "wide", wideButton);
    // listen
    listenButton.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(listenButton);
    listenAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "listen", listenButton);
    // gr meter
    addAndMakeVisible(meter);
    grLabel.setText("GR", juce::NotificationType::dontSendNotification);
    grLabel.setLookAndFeel(&deesserLookAndFeel);
    addAndMakeVisible(grLabel);
    setSize(260, 420);
}

DeesserAudioProcessorEditor::~DeesserAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DeesserAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw powerlines
    powerLine.drawPowerLine(g, 117.0f, 10.0f, 110.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 100.0f, 30.0f, 4, 0, "De-esser");
}

void DeesserAudioProcessorEditor::resized()
{
    int col1XPosition = 40;
    int col2XPosition = 110;
    int col3XPosition = 175;
    int knobWidth = 50;
    thresholdSlider.setBounds(col1XPosition, 60, knobWidth, knobWidth);
    thresholdLabel.setBounds(thresholdSlider.getX(), thresholdSlider.getBottom(), knobWidth, 20);
    crossoverSlider.setBounds(col1XPosition, 150, knobWidth, knobWidth);
    crossoverLabel.setBounds(crossoverSlider.getX(), crossoverSlider.getBottom(), knobWidth, 20);
    attackSlider.setBounds(col1XPosition, 240, knobWidth, knobWidth);
    attackLabel.setBounds(attackSlider.getX(), attackSlider.getBottom(), knobWidth, 20);
    releaseSlider.setBounds(col1XPosition, 330, knobWidth, knobWidth);
    releaseLabel.setBounds(releaseSlider.getX(), releaseSlider.getBottom(), knobWidth, 20);
    stereoButton.setBounds(col2XPosition, 125, knobWidth, 20);
    wideButton.setBounds(col2XPosition, 215, knobWidth, 20);
    listenButton.setBounds(col2XPosition, 305, knobWidth, 20);
    meter.setBounds(col3XPosition, 113, meter.getMeterWidth(), meter.getMeterHeight());
    grLabel.setBounds(meter.getX() + 5, meter.getY() - 15, 44, 20);
}
