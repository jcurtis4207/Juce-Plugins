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
    : AudioProcessorEditor(&p), audioProcessor(p), 
    grMeter(audioProcessor.gainReductionLeft, audioProcessor.gainReductionRight)
{
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(thresholdKnob);
    addAndMakeVisible(crossoverKnob);
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(releaseKnob);
    addAndMakeVisible(stereoButton);
    addAndMakeVisible(wideButton);
    addAndMakeVisible(listenButton);
    addAndMakeVisible(grMeter);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "threshold", thresholdKnob);
    crossoverAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "crossoverFreq", crossoverKnob);
    attackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "attack", attackKnob);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "release", releaseKnob);
    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "stereo", stereoButton);
    wideAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "wide", wideButton);
    listenButton.onClick = [&]() { audioProcessor.listen = listenButton.getToggleState(); };
    setSize(260, 420);
}

DeesserAudioProcessorEditor::~DeesserAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DeesserAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 260, 50);
    const int col1XPosition = 40;
    const int col2XPosition = 115;
    const int knobWidth = 50;
    thresholdKnob.setBounds(col1XPosition, 60, knobWidth, knobWidth + 25);
    crossoverKnob.setBounds(col1XPosition, 150, knobWidth, knobWidth + 25);
    attackKnob.setBounds(col1XPosition, 240, knobWidth, knobWidth + 25);
    releaseKnob.setBounds(col1XPosition, 330, knobWidth, knobWidth + 25);
    stereoButton.setBounds(col2XPosition, 125, knobWidth, knobWidth);
    wideButton.setBounds(col2XPosition, 215, knobWidth, knobWidth);
    listenButton.setBounds(col2XPosition, 305, knobWidth, knobWidth);
    grMeter.setBounds(185, 108, grMeter.getMeterWidth(), grMeter.getMeterHeight());
}
