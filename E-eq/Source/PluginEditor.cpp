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
    // setup labels
    for (int i = 0; i < 14; i++)
    {
        knobLabels[i].setText(knobLabelText[i], juce::NotificationType::dontSendNotification);
        knobLabels[i].setFont(12.0f);
        knobLabels[i].setColour(juce::Label::ColourIds::textColourId, juce::Colours::grey);
        knobLabels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(knobLabels[i]);
    }
    // setup filter knobs
    for (int i = 0; i < 4; i++)
    {
        filterKnobs[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        filterKnobs[i].setLookAndFeel(&eqLookAndFeel);
        if (i == 0 || i == 2)
        {
            filterKnobs[i].setTextValueSuffix(" Hz");
        }
        else
        {
            // decrease slope knob range
            filterKnobs[i].setRotaryParameters(-0.4f, 0.4f, true);
            // decrease drag distance
            filterKnobs[i].setMouseDragSensitivity(25);
        }
        addAndMakeVisible(filterKnobs[i]);
        filterKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, filterParamIDs[i], filterKnobs[i]);
        filterKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[4]);
        filterKnobs[i].setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::black);
    }
    // setup parametric bands
    for (int i = 0; i < 10; i++)
    {
        bandKnobs[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        bandKnobs[i].setLookAndFeel(&eqLookAndFeel);
        bandKnobs[i].setTextValueSuffix(bandSuffixes[i]);
        addAndMakeVisible(bandKnobs[i]);
        bandKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, bandParamIDs[i], bandKnobs[i]);
        // set knob color by band
        if (i < 2)
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[0]);
        }
        else if (i > 7)
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[3]);
        }
        else if (i < 5)
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[1]);
        }
        else
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[2]);
        }
        // set outline color
        bandKnobs[i].setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::white);
    }
    // setup filter bypass buttons
    hpfBypassButton.setLookAndFeel(&eqLookAndFeel);
    addAndMakeVisible(hpfBypassButton);
    hpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "hpfBypass", hpfBypassButton);
    lpfBypassButton.setLookAndFeel(&eqLookAndFeel);
    addAndMakeVisible(lpfBypassButton);
    lpfBypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "lpfBypass", lpfBypassButton);
    // setup shelf/bell buttons
    band1BellButton.setLookAndFeel(&eqLookAndFeel);
    addAndMakeVisible(band1BellButton);
    band1BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "band1Bell", band1BellButton);
    band4BellButton.setLookAndFeel(&eqLookAndFeel);
    addAndMakeVisible(band4BellButton);
    band4BellAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "band4Bell", band4BellButton);
    // set plugin size
    setSize(870, 255);
}

EeqAudioProcessorEditor::~EeqAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void EeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff303030));
    // draw powerlines
    powerLine.drawPowerLine(g, 85.0f, 10.0f, 110.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 70.0f, 30.0f, 4, 0, "E-EQ");
    // calculate divider lines
    float xPosition;
    float bigOffset = 50.0f;
    float smallOffset = 6.0f;
    float yPosition[6] = { 54.0f, 60.0f, 120.0f, 160.0f, 220.0f, 226.0f };
    juce::Path p;
    for (int i = 0; i < 3; i++)
    {
        xPosition = i * 100.0f + 75.0f;
        p.startNewSubPath(xPosition + smallOffset, yPosition[0]);
        p.lineTo(xPosition, yPosition[1]);
        p.lineTo(xPosition, yPosition[2]);
        p.lineTo(xPosition - bigOffset, yPosition[3]);
        p.lineTo(xPosition - bigOffset, yPosition[4]);
        p.lineTo(xPosition - bigOffset + smallOffset, yPosition[5]);
    }
    xPosition = 425;
    p.startNewSubPath(xPosition - smallOffset, yPosition[0]);
    p.lineTo(xPosition, yPosition[1]);
    p.lineTo(xPosition, yPosition[4]);
    p.lineTo(xPosition - smallOffset, yPosition[5]);
    p.startNewSubPath(xPosition + smallOffset, yPosition[0]);
    p.lineTo(xPosition, yPosition[1]);
    p.lineTo(xPosition, yPosition[4]);
    p.lineTo(xPosition + smallOffset, yPosition[5]);
    for (int i = 0; i < 3; i++)
    {
        xPosition = i * 100.0f + 575.0f;
        p.startNewSubPath(xPosition - smallOffset, yPosition[0]);
        p.lineTo(xPosition, yPosition[1]);
        p.lineTo(xPosition, yPosition[2]);
        p.lineTo(xPosition + bigOffset, yPosition[3]);
        p.lineTo(xPosition + bigOffset, yPosition[4]);
        p.lineTo(xPosition + bigOffset - smallOffset, yPosition[5]);
    }
    // draw divider lines
    g.setColour(juce::Colours::grey);
    g.strokePath(p, { 1, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
}

void EeqAudioProcessorEditor::resized()
{
    int knobWidth = 50;
    int topRowYPosition = 70;
    int bottomRowYPosition = 150;
    int startXPosition = 150;
    // parametric knobs
    bandKnobs[0].setBounds(startXPosition, bottomRowYPosition, knobWidth, knobWidth);           // band 1 freq
    bandKnobs[1].setBounds(startXPosition + 50, topRowYPosition, knobWidth, knobWidth);         // band 1 gain
    band1BellButton.setBounds(startXPosition, bottomRowYPosition + 70, knobWidth, 20);
    bandKnobs[2].setBounds(startXPosition + 200, bottomRowYPosition, knobWidth, knobWidth);     // band 2 freq
    bandKnobs[3].setBounds(startXPosition + 150, topRowYPosition, knobWidth, knobWidth);        // band 2 gain
    bandKnobs[4].setBounds(startXPosition + 100, bottomRowYPosition, knobWidth, knobWidth);     // band 2 q
    bandKnobs[5].setBounds(startXPosition + 400, bottomRowYPosition, knobWidth, knobWidth);     // band 3 freq
    bandKnobs[6].setBounds(startXPosition + 350, topRowYPosition, knobWidth, knobWidth);        // band 3 gain
    bandKnobs[7].setBounds(startXPosition + 300, bottomRowYPosition, knobWidth, knobWidth);     // band 3 q
    bandKnobs[8].setBounds(startXPosition + 500, bottomRowYPosition, knobWidth, knobWidth);     // band 4 freq
    bandKnobs[9].setBounds(startXPosition + 450, topRowYPosition, knobWidth, knobWidth);        // band 4 gain
    band4BellButton.setBounds(startXPosition + 500, bottomRowYPosition + 70, knobWidth, 20);
    // filter knobs
    filterKnobs[0].setBounds(startXPosition - 100, bottomRowYPosition, knobWidth, knobWidth);   // hpf freq
    filterKnobs[1].setBounds(startXPosition - 50, topRowYPosition, knobWidth, knobWidth);       // hpf slope
    filterKnobs[2].setBounds(startXPosition + 600, bottomRowYPosition, knobWidth, knobWidth);   // lpf freq
    filterKnobs[3].setBounds(startXPosition + 550, topRowYPosition, knobWidth, knobWidth);      // lpf slope
    hpfBypassButton.setBounds(startXPosition - 100, bottomRowYPosition + 70, knobWidth, 20);
    lpfBypassButton.setBounds(startXPosition + 600, bottomRowYPosition + 70, knobWidth, 20);
    // labels
    for (int i = 0; i < 10; i++)
    {
        knobLabels[i].setBounds(bandKnobs[i].getX(), bandKnobs[i].getBottom(), knobWidth, 12);
    }
    for (int i = 0; i < 4; i++)
    {
        knobLabels[i + 10].setBounds(filterKnobs[i].getX(), filterKnobs[i].getBottom(), knobWidth, 12);
    }
}