/*
*   MultiBandComp Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

MultiBandCompAudioProcessorEditor::MultiBandCompAudioProcessorEditor(MultiBandCompAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), grMeters{
	GainReductionMeter(audioProcessor.gainReduction[0]),
	GainReductionMeter(audioProcessor.gainReduction[1]),
	GainReductionMeter(audioProcessor.gainReduction[2]),
	GainReductionMeter(audioProcessor.gainReduction[3])
}
{
	addAndMakeVisible(bgImage);
	addAndMakeVisible(powerLine);
	addAndMakeVisible(listenLabel);
	// frequency controls
	for (int crossover = 0; crossover < numBands - 1; crossover++)
	{
		addAndMakeVisible(freqKnobs[crossover]);
	}
	freqAttach[0] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "crossoverFreqB", freqKnobs[0]);
	freqAttach[1] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "crossoverFreqA", freqKnobs[1]);
	freqAttach[2] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
		audioProcessor.parameters, "crossoverFreqC", freqKnobs[2]);
	// compression controls
	for (int band = 0; band < numBands; band++)
	{
		const auto bandNum = juce::String(band + 1);
		addAndMakeVisible(bandLabels[band]);
		addAndMakeVisible(grMeters[band]);
		addAndMakeVisible(ratioKnobs[band]);
		addAndMakeVisible(thresholdKnobs[band]);
		addAndMakeVisible(attackKnobs[band]);
		addAndMakeVisible(releaseKnobs[band]);
		addAndMakeVisible(makeUpKnobs[band]);
		addAndMakeVisible(listenButtons[band]);
		thresholdAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
			audioProcessor.parameters, "threshold" + bandNum, thresholdKnobs[band]);
		ratioAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
			audioProcessor.parameters, "ratio" + bandNum, ratioKnobs[band]);
		attackAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
			audioProcessor.parameters, "attack" + bandNum, attackKnobs[band]);
		releaseAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
			audioProcessor.parameters, "release" + bandNum, releaseKnobs[band]);
		makeUpAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
			audioProcessor.parameters, "makeUp" + bandNum, makeUpKnobs[band]);
	}
	addAndMakeVisible(stereoButton);
	stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
		audioProcessor.parameters, "stereo", stereoButton);
	listenButtons[0].onClick = [&]() { audioProcessor.listen[0] = listenButtons[0].getToggleState(); };
	listenButtons[1].onClick = [&]() { audioProcessor.listen[1] = listenButtons[1].getToggleState(); };
	listenButtons[2].onClick = [&]() { audioProcessor.listen[2] = listenButtons[2].getToggleState(); };
	listenButtons[3].onClick = [&]() { audioProcessor.listen[3] = listenButtons[3].getToggleState(); };
	setSize(730, 480);
}

MultiBandCompAudioProcessorEditor::~MultiBandCompAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void MultiBandCompAudioProcessorEditor::resized()
{
	bgImage.setBounds(getLocalBounds());
	powerLine.setBounds(0, 10, 300, 50);
	for (int band = 0; band < numBands; band++)
	{
		ratioKnobs[band].setBounds(band * 170 + 40, 170, 80, 120);
		thresholdKnobs[band].setBounds(ratioKnobs[band].getInnerArea());
		attackKnobs[band].setBounds(ratioKnobs[band].getX() - 10, ratioKnobs[band].getBottom() - 25, 40, 70);
		releaseKnobs[band].setBounds(attackKnobs[band].getRight() + 20, attackKnobs[band].getY(), 40, 70);
		makeUpKnobs[band].setBounds(attackKnobs[band].getX() + 30, attackKnobs[band].getBottom(), 40, 70);
		grMeters[band].setBounds(ratioKnobs[band].getRight() + 20, ratioKnobs[band].getY() - 10, 
			grMeters[band].getMeterWidth(), grMeters[band].getMeterHeight());
		bandLabels[band].setBounds(ratioKnobs[band].getX() - 10, ratioKnobs[band].getY() - 30, 160, 15);
		listenButtons[band].setBounds(makeUpKnobs[band].getX() - 5, makeUpKnobs[band].getBottom() + 30, 50, 50);
	}
	for (int crossover = 0; crossover < numBands - 1; crossover++)
	{
		freqKnobs[crossover].setBounds(ratioKnobs[crossover + 1].getX() - 40, 60, 50, 80);
	}
	stereoButton.setBounds(freqKnobs[2].getRight() + 50, freqKnobs[2].getY() + 15, 50, 60);
	const int xPos = listenButtons[0].getX() + (listenButtons[0].getWidth() / 2) - 1;
	const int width = listenButtons[3].getX() + (listenButtons[3].getWidth() / 2) - xPos + 1;
	listenLabel.setBounds(xPos, listenButtons[0].getY() - 20, width, 13);
}
