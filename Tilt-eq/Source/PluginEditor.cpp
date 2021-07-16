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
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(freqKnob);
    addAndMakeVisible(tiltKnob);
    freqAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "freq", freqKnob);
    gainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", tiltKnob);
    setSize(235, 150);
}

TilteqAudioProcessorEditor::~TilteqAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void TilteqAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 235, 50);
    freqKnob.setBounds(40, 60, 50, 75);
    tiltKnob.setBounds(145, 60, 50, 75);
}
