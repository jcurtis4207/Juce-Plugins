/*
*   Reverb Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ReverbAudioProcessorEditor::ReverbAudioProcessorEditor(ReverbAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	addAndMakeVisible(bgImage);
	addAndMakeVisible(powerLine);
	addAndMakeVisible(filterLabel);
	addAndMakeVisible(modLabel);
	addAndMakeVisible(sizeKnob);
	addAndMakeVisible(dampingKnob);
	addAndMakeVisible(mixKnob);
	addAndMakeVisible(predelayKnob);
	addAndMakeVisible(depthKnob);
	addAndMakeVisible(rateKnob);
	addAndMakeVisible(hpfKnob);
	addAndMakeVisible(lpfKnob);
	sizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "roomSize", sizeKnob);
	dampingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "damping", dampingKnob);
	mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixKnob);
	predelayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "predelay", predelayKnob);
	hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "hpfFreq", hpfKnob);
	lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lpfFreq", lpfKnob);
	depthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "modDepth", depthKnob);
	rateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "modRate", rateKnob);
	setSize(470, 250);
}

ReverbAudioProcessorEditor::~ReverbAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void ReverbAudioProcessorEditor::resized()
{
	bgImage.setBounds(getLocalBounds());
	powerLine.setBounds(0, 10, 250, 50);
	int knobWidth = 50;
	sizeKnob.setBounds(20, 80, 120, 150);
	dampingKnob.setBounds(sizeKnob.getRight() + 20, sizeKnob.getY() + 35, knobWidth, knobWidth + 25);
	predelayKnob.setBounds(dampingKnob.getRight() + 20, sizeKnob.getY() - 10, knobWidth, knobWidth + 25);
	mixKnob.setBounds(dampingKnob.getRight() + 20, sizeKnob.getY() + 80, knobWidth, knobWidth + 25);
	hpfKnob.setBounds(predelayKnob.getRight() + 30, predelayKnob.getY(), knobWidth, knobWidth + 25);
	lpfKnob.setBounds(hpfKnob.getX(), mixKnob.getY(), knobWidth, knobWidth + 25);
	filterLabel.setBounds(hpfKnob.getX() - 10, hpfKnob.getY() - 25, knobWidth + 20, 13);
	depthKnob.setBounds(hpfKnob.getRight() + 30, hpfKnob.getY(), knobWidth, knobWidth + 25);
	rateKnob.setBounds(depthKnob.getX(), lpfKnob.getY(), knobWidth, knobWidth + 25);
	modLabel.setBounds(depthKnob.getX() - 10, depthKnob.getY() - 25, knobWidth + 20, 13);
}
