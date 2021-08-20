/*
*   E-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

EeqAudioProcessorEditor::EeqAudioProcessorEditor(EeqAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(multiLabel);
    // setup filter knobs
    for (int band = 0; band < numFilters; band++)
    {
        // decrease slope knob range and drag distance
        slopeKnobs[band].setRotaryParameters(-0.8f, 0.8f, true);
        slopeKnobs[band].setMouseDragSensitivity(50);
        addAndMakeVisible(slopeKnobs[band]);
        filterKnobsAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, filterParamIDs[band + 2], slopeKnobs[band]);
        addAndMakeVisible(filterKnobs[band]);
        filterKnobsAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, filterParamIDs[band], filterKnobs[band]);
    }
    // setup parametric bands
    for (int band = 0; band < numBands; band++)
    {
        addAndMakeVisible(freqKnobs[band]);
        addAndMakeVisible(gainKnobs[band]);
        addAndMakeVisible(qKnobs[band]);
        juce::String bandNum = "band" + juce::String(band + 1);
        bandKnobsAttach[band] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, bandNum + "Freq", freqKnobs[band]);
        bandKnobsAttach[band+4] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, bandNum + "Gain", gainKnobs[band]);
        bandKnobsAttach[band+8] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, bandNum + "Q", qKnobs[band]);
    }
    // setup buttons
    addAndMakeVisible(hpfBypassButton);
    addAndMakeVisible(lpfBypassButton);
    addAndMakeVisible(band1BellButton);
    addAndMakeVisible(band4BellButton);
    hpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "hpfBypass", hpfBypassButton);
    lpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "lpfBypass", lpfBypassButton);
    band1BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "band1Bell", band1BellButton);
    band4BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "band4Bell", band4BellButton);
    setSize(620, 310);
}

EeqAudioProcessorEditor::~EeqAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void EeqAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 260, 50);
    const int smallKnobWidth = 50;
    const int bigKnobWidth = 80;
    const int knobOffset = (bigKnobWidth - smallKnobWidth) / 2;
    const int yPosition = 70;
    // filter knobs
    slopeKnobs[0].setBounds(20, yPosition + 50, bigKnobWidth, bigKnobWidth + 30);
    slopeKnobs[1].setBounds(20 + 500, yPosition + 50, bigKnobWidth, bigKnobWidth + 30);
    filterKnobs[0].setBounds(slopeKnobs[0].getInnerArea());
    filterKnobs[1].setBounds(slopeKnobs[1].getInnerArea());
    // band knobs
    for (int band = 0; band < numBands; band++)
    {
        freqKnobs[band].setBounds(band * 100 + 120, yPosition, bigKnobWidth, bigKnobWidth + 30);
        gainKnobs[band].setBounds(freqKnobs[band].getInnerArea());
        qKnobs[band].setBounds(band * 100 + 120 + knobOffset, yPosition + 110, smallKnobWidth, smallKnobWidth + 30);
    }
    // multilabel
    const int xPos = qKnobs[0].getX() + (qKnobs[0].getWidth() / 2) - 1;
    const int width = qKnobs[3].getX() + (qKnobs[3].getWidth() / 2) - xPos + 1;
    multiLabel.setBounds(xPos, qKnobs[0].getY() - 20, width, 13);
    // buttons
    hpfBypassButton.setBounds(filterKnobs[0].getX(), filterKnobs[0].getY() - 70, smallKnobWidth, smallKnobWidth);
    lpfBypassButton.setBounds(filterKnobs[1].getX(), filterKnobs[1].getY() - 70, smallKnobWidth, smallKnobWidth);
    band1BellButton.setBounds(gainKnobs[0].getX(), qKnobs[0].getY() + 70, smallKnobWidth, smallKnobWidth);
    band4BellButton.setBounds(gainKnobs[3].getX(), qKnobs[3].getY() + 70, smallKnobWidth, smallKnobWidth);
}