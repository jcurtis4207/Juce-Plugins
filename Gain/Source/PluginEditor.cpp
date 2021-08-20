/*
*   Gain Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

GainAudioProcessorEditor::GainAudioProcessorEditor(GainAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), 
    meter(audioProcessor.bufferMagnitudeL, audioProcessor.bufferMagnitudeR)
{
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(gainKnob);
    addAndMakeVisible(phaseButton);
    addAndMakeVisible(meter);
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "gain", gainKnob);
    phaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "phase", phaseButton);
    setSize(225, 300);
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GainAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 225, 50);
    gainKnob.setBounds(30, 80, 60, 120);
    phaseButton.setBounds(35, 200, 50, 60);
    meter.setBounds(130, 50, meter.getMeterWidth(), meter.getMeterHeight());
}
