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
	GainReductionMeter(audioProcessor.gainReduction[0], audioProcessor.gainReduction[1]),
	GainReductionMeter(audioProcessor.gainReduction[2], audioProcessor.gainReduction[3]),
	GainReductionMeter(audioProcessor.gainReduction[4], audioProcessor.gainReduction[5]),
	GainReductionMeter(audioProcessor.gainReduction[6], audioProcessor.gainReduction[7])
}
{
	addAndMakeVisible(bgImage);
	addAndMakeVisible(powerLine);
	// frequency controls
	for (int i = 0; i < 3; i++)
	{
		addAndMakeVisible(freqKnobs[i]);
	}
	freqAttach[0] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crossoverFreqB", freqKnobs[0]);
	freqAttach[1] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crossoverFreqA", freqKnobs[1]);
	freqAttach[2] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "crossoverFreqC", freqKnobs[2]);
	// compression controls
	for (int band = 0; band < 4; band++)
	{
		auto bandNum = juce::String(band + 1);
		addAndMakeVisible(bandLabels[band]);
		addAndMakeVisible(grMeters[band]);
		addAndMakeVisible(ratioKnobs[band]);
		addAndMakeVisible(thresholdKnobs[band]);
		addAndMakeVisible(attackKnobs[band]);
		addAndMakeVisible(releaseKnobs[band]);
		addAndMakeVisible(makeUpKnobs[band]);
		thresholdAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold" + bandNum, thresholdKnobs[band]);
		ratioAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ratio" + bandNum, ratioKnobs[band]);
		attackAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "attack" + bandNum, attackKnobs[band]);
		releaseAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release" + bandNum, releaseKnobs[band]);
		makeUpAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "makeUp" + bandNum, makeUpKnobs[band]);
	}
	addAndMakeVisible(stereoButton);
	stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stereo", stereoButton);
	setSize(730, 430);
}


MultiBandCompAudioProcessorEditor::~MultiBandCompAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void MultiBandCompAudioProcessorEditor::resized()
{
	bgImage.setBounds(getLocalBounds());
	powerLine.setBounds(0, 10, 300, 50);
	for (int band = 0; band < 4; band++)
	{
		ratioKnobs[band].setBounds(band * 170 + 40, 170, 80, 120);
		thresholdKnobs[band].setBounds(ratioKnobs[band].getInnerArea());
		attackKnobs[band].setBounds(ratioKnobs[band].getX() - 10, ratioKnobs[band].getBottom() - 25, 40, 70);
		releaseKnobs[band].setBounds(attackKnobs[band].getRight() + 20, attackKnobs[band].getY(), 40, 70);
		makeUpKnobs[band].setBounds(attackKnobs[band].getX() + 30, attackKnobs[band].getBottom(), 40, 70);
		grMeters[band].setBounds(ratioKnobs[band].getRight() + 10, ratioKnobs[band].getY() - 10, grMeters[band].getMeterWidth(), grMeters[band].getMeterHeight());
		bandLabels[band].setBounds(ratioKnobs[band].getX() - 10, ratioKnobs[band].getY() - 30, 160, 15);
	}
	for (int i = 0; i < 3; i++)
	{
		freqKnobs[i].setBounds(ratioKnobs[i + 1].getX() - 40, 60, 50, 80);
	}
	stereoButton.setBounds(freqKnobs[2].getRight() + 50, freqKnobs[2].getY() + 15, 50, 60);
}
