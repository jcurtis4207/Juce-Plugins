/*
*   Delay Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

DelayAudioProcessorEditor::DelayAudioProcessorEditor(DelayAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	addAndMakeVisible(bgImage);
	addAndMakeVisible(powerLine);
	addAndMakeVisible(modLabel);
	addAndMakeVisible(filterLabel);
	addAndMakeVisible(delayKnob);
	addAndMakeVisible(subdivisionKnob);
	addAndMakeVisible(feedbackKnob);
	addAndMakeVisible(mixKnob);
	addAndMakeVisible(widthKnob);
	addAndMakeVisible(driveKnob);
	addAndMakeVisible(hpfKnob);
	addAndMakeVisible(lpfKnob);
	addAndMakeVisible(depthKnob);
	addAndMakeVisible(rateKnob);
	addAndMakeVisible(syncButton);
	delayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "delayTime", delayKnob);
	subdivisionAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "subdivisionIndex", subdivisionKnob);
	feedbackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "feedback", feedbackKnob);
	mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "mix", mixKnob);
	widthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "width", widthKnob);
	driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "drive", driveKnob);
	hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "hpfFreq", hpfKnob);
	lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "lpfFreq", lpfKnob);
	depthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "modDepth", depthKnob);
	rateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "modRate", rateKnob);
	syncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.parameters, "bpmSync", syncButton);
	syncButton.onClick = [&]() { switchKnob(syncButton.getToggleState()); };
	switchKnob(syncButton.getToggleState());
	setSize(490, 240);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void DelayAudioProcessorEditor::resized()
{
	bgImage.setBounds(getLocalBounds());
	powerLine.setBounds(0, 10, 250, 50);
	const int knobWidth = 40;
	delayKnob.setBounds(140, 70, 120, 145);
	subdivisionKnob.setBounds(delayKnob.getBounds());
	feedbackKnob.setBounds(delayKnob.getX() - 20 - knobWidth, delayKnob.getY() - 5, knobWidth, knobWidth + 25);
	mixKnob.setBounds(feedbackKnob.getX(), delayKnob.getY() + delayKnob.getWidth() - knobWidth, knobWidth, knobWidth + 25);
	widthKnob.setBounds(delayKnob.getRight() + 20, feedbackKnob.getY(), knobWidth, knobWidth + 25);
	driveKnob.setBounds(widthKnob.getX(), mixKnob.getY(), knobWidth, knobWidth + 25);
	hpfKnob.setBounds(widthKnob.getRight() + 30, widthKnob.getY(), knobWidth, knobWidth + 25);
	lpfKnob.setBounds(hpfKnob.getX(), driveKnob.getY(), knobWidth, knobWidth + 25);
	filterLabel.setBounds(hpfKnob.getX() - 10, hpfKnob.getY() - 25, knobWidth + 20, 13);
	depthKnob.setBounds(hpfKnob.getRight() + 30, hpfKnob.getY(), knobWidth, knobWidth + 25);
	rateKnob.setBounds(depthKnob.getX(), lpfKnob.getY(), knobWidth, knobWidth + 25);
	modLabel.setBounds(depthKnob.getX() - 10, depthKnob.getY() - 25, knobWidth + 20, 13);
	syncButton.setBounds(20, delayKnob.getY() + 45, knobWidth, knobWidth + 10);
}

// show delay control based on sync status
void DelayAudioProcessorEditor::switchKnob(bool isSync)
{
	if (isSync)
	{
		delayKnob.setVisible(false);
		subdivisionKnob.setVisible(true);
	}
	else
	{
		delayKnob.setVisible(true);
		subdivisionKnob.setVisible(false);
	}
}