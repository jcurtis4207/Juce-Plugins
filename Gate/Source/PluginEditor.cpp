/*
*   Gate Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

GateAudioProcessorEditor::GateAudioProcessorEditor(GateAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), 
	grMeter(audioProcessor.gainReductionLeft, audioProcessor.gainReductionRight)
{
	addAndMakeVisible(bgImage);
	addAndMakeVisible(powerLine);
	addAndMakeVisible(thresholdKnob);
	addAndMakeVisible(ratioKnob);
	addAndMakeVisible(attackKnob);
	addAndMakeVisible(releaseKnob);
	addAndMakeVisible(holdKnob);
	addAndMakeVisible(hpfKnob);
	addAndMakeVisible(lpfKnob);
	addAndMakeVisible(scEnableButton);
	addAndMakeVisible(listenButton);
	addAndMakeVisible(grMeter);
	thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "threshold", thresholdKnob);
	ratioAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "ratio", ratioKnob);
	attackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "attack", attackKnob);
	releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "release", releaseKnob);
	holdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "hold", holdKnob);
	hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "hpfFreq", hpfKnob);
	lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "lpfFreq", lpfKnob);
	scEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.parameters, "filterEnable", scEnableButton);
	listenButton.onClick = [&]() { audioProcessor.listen = listenButton.getToggleState(); };
	setSize(300, 480);
}

GateAudioProcessorEditor::~GateAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void GateAudioProcessorEditor::resized()
{
	bgImage.setBounds(getLocalBounds());
	powerLine.setBounds(0, 10, 250, 50);
	grMeter.setBounds(20, 55, grMeter.getMeterWidth(), grMeter.getMeterHeight());
	thresholdKnob.setBounds(80, 75, 70, 100);
	ratioKnob.setBounds(thresholdKnob.getX(), thresholdKnob.getBottom() + 30, 70, 100);
	attackKnob.setBounds(thresholdKnob.getRight() + 30, 60, 50, 80);
	releaseKnob.setBounds(attackKnob.getX(), attackKnob.getBottom() + 10, 50, 80);
	holdKnob.setBounds(releaseKnob.getX(), releaseKnob.getBottom() + 10, 50, 80);
	lpfKnob.setBounds(holdKnob.getX(), holdKnob.getBottom() + 10, 50, 80);
	hpfKnob.setBounds(ratioKnob.getX() + 10, lpfKnob.getY(), 50, 80);
	scEnableButton.setBounds(hpfKnob.getX(), hpfKnob.getBottom() + 10, 50, 50);
	listenButton.setBounds(lpfKnob.getX(), scEnableButton.getY(), 50, 50);
}
