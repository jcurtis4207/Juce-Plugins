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
    for (int i = 0; i < 2; i++)
    {
        // decrease slope knob range and drag distance
        slopeKnobs[i].setRotaryParameters(-0.8f, 0.8f, true);
        slopeKnobs[i].setMouseDragSensitivity(50);
        addAndMakeVisible(slopeKnobs[i]);
        filterKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, filterParamIDs[i + 2], slopeKnobs[i]);
        filterKnobs[i].setTextValueSuffix(" Hz");
        addAndMakeVisible(filterKnobs[i]);
        filterKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, filterParamIDs[i], filterKnobs[i]);
    }
    // setup parametric bands
    for (int i = 0; i < 4; i++)
    {
        freqKnobs[i].setTextValueSuffix(" Hz");
        gainKnobs[i].setTextValueSuffix(" dB");
        qKnobs[i].setTextValueSuffix(" Q");
        addAndMakeVisible(freqKnobs[i]);
        addAndMakeVisible(gainKnobs[i]);
        addAndMakeVisible(qKnobs[i]);
        juce::String bandNum = "band" + juce::String(i + 1);
        bandKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, bandNum + "Freq", freqKnobs[i]);
        bandKnobsAttach[i+4] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, bandNum + "Gain", gainKnobs[i]);
        bandKnobsAttach[i+8] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, bandNum + "Q", qKnobs[i]);
    }
    // setup buttons
    addAndMakeVisible(hpfBypassButton);
    addAndMakeVisible(lpfBypassButton);
    addAndMakeVisible(band1BellButton);
    addAndMakeVisible(band4BellButton);
    hpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "hpfBypass", hpfBypassButton);
    lpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "lpfBypass", lpfBypassButton);
    band1BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "band1Bell", band1BellButton);
    band4BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "band4Bell", band4BellButton);
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
    int smallKnobWidth = 50;
    int bigKnobWidth = 80;
    int knobOffset = (bigKnobWidth - smallKnobWidth) / 2;
    int yPosition = 70;
    // filter knobs
    slopeKnobs[0].setBounds(20, yPosition + 50, bigKnobWidth, bigKnobWidth + 30);
    slopeKnobs[1].setBounds(20 + 500, yPosition + 50, bigKnobWidth, bigKnobWidth + 30);
    filterKnobs[0].setBounds(slopeKnobs[0].getInnerArea());
    filterKnobs[1].setBounds(slopeKnobs[1].getInnerArea());
    // band knobs
    for (int i = 0; i < 4; i++)
    {
        freqKnobs[i].setBounds(i * 100 + 120, yPosition, bigKnobWidth, bigKnobWidth + 30);
        gainKnobs[i].setBounds(freqKnobs[i].getInnerArea());
        qKnobs[i].setBounds(i * 100 + 120 + knobOffset, yPosition + 110, smallKnobWidth, smallKnobWidth + 30);
    }
    // multilabel
    int xPos = qKnobs[0].getX() + (qKnobs[0].getWidth() / 2) - 1;
    int width = qKnobs[3].getX() + (qKnobs[3].getWidth() / 2) - xPos + 1;
    multiLabel.setBounds(xPos, qKnobs[0].getY() - 20, width, 13);
    // buttons
    hpfBypassButton.setBounds(filterKnobs[0].getX(), filterKnobs[0].getY() - 70, smallKnobWidth, smallKnobWidth);
    lpfBypassButton.setBounds(filterKnobs[1].getX(), filterKnobs[1].getY() - 70, smallKnobWidth, smallKnobWidth);
    band1BellButton.setBounds(gainKnobs[0].getX(), qKnobs[0].getY() + 70, smallKnobWidth, smallKnobWidth);
    band4BellButton.setBounds(gainKnobs[3].getX(), qKnobs[3].getY() + 70, smallKnobWidth, smallKnobWidth);
}