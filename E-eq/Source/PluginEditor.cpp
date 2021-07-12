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
    // setup filter knobs
    for (int i = 0; i < 4; i++)
    {
        filterKnobs[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        filterKnobs[i].setLookAndFeel(&eqLookAndFeel);
        if (i % 2 == 0)
        {
            filterKnobs[i].setTextValueSuffix(" Hz");
            // set background
            filterKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[4]);
        }
        else
        {
            // decrease slope knob range
            filterKnobs[i].setRotaryParameters(-0.8f, 0.8f, true);
            // decrease drag distance
            filterKnobs[i].setMouseDragSensitivity(50);
            // set darker background
            filterKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[5]);
        }
        addAndMakeVisible(filterKnobs[i]);
        filterKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, filterParamIDs[i], filterKnobs[i]);
        filterKnobs[i].setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::black);
    }
    // setup parametric bands
    for (int i = 0; i < 12; i++)
    {
        int bandNum = i / 3 + 1;
        bandKnobs[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        bandKnobs[i].setLookAndFeel(&eqLookAndFeel);
        bandKnobs[i].setTextValueSuffix(bandSuffixes[i % 3]);
        addAndMakeVisible(bandKnobs[i]);
        juce::String paramID = "band" + juce::String(bandNum) + bandParamIDSuffixes[i % 3];
        bandKnobsAttach[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, paramID, bandKnobs[i]);
        // set outline color
        bandKnobs[i].setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::white);
        // set knob color by band - darken outer knobs
        if (i % 3 == 0)
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[bandNum - 1].darker());
        }
        else
        {
            bandKnobs[i].setColour(juce::Slider::ColourIds::thumbColourId, bandColors[bandNum - 1]);
        }
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
    setSize(620, 300);
}

EeqAudioProcessorEditor::~EeqAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

void EeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw powerlines
    powerLine.drawPowerLine(g, 87.0f, 10.0f, 110.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 70.0f, 30.0f, 4, 0, "E-EQ");
    // draw knob diagrams
    drawKnobDiagram(g, "Gain", "Freq", 310, 45);
    drawKnobDiagram(g, "Freq", "Slope", 60, 220);
    drawKnobDiagram(g, "Freq", "Slope", 560, 220);
    // draw Q label and lines
    float leftX = 160.0f;
    float rightX = 460.0f;
    float centerX = leftX + ((rightX - leftX) / 2.0f);
    juce::Path p;
    p.startNewSubPath(leftX, 175.0f);
    p.lineTo(leftX, 170.0f);
    p.lineTo(centerX - 7, 170.0f);
    p.startNewSubPath(centerX + 7, 170.0f);
    p.lineTo(rightX, 170.0f);
    p.lineTo(rightX, 175.0f);
    g.setColour(juce::Colours::grey);
    g.strokePath(p, { 1, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
    g.drawText("Q", (int)centerX - 4, 166, 8, 8, juce::Justification::centred, false);
}

void EeqAudioProcessorEditor::resized()
{
    int smallKnobWidth = 50;
    int bigKnobWidth = 80;
    int knobOffset = (bigKnobWidth - smallKnobWidth) / 2;
    int yPosition = 70;
    // filter knobs
    filterKnobs[1].setBounds(20, yPosition + 50, bigKnobWidth, bigKnobWidth);
    filterKnobs[0].setBounds(filterKnobs[1].getX() + knobOffset, yPosition + knobOffset + 50, smallKnobWidth, smallKnobWidth);
    filterKnobs[0].setAlwaysOnTop(true);
    filterKnobs[3].setBounds(20 + 500, yPosition + 50, bigKnobWidth, bigKnobWidth);
    filterKnobs[2].setBounds(filterKnobs[3].getX() + knobOffset, yPosition + knobOffset + 50, smallKnobWidth, smallKnobWidth);
    filterKnobs[2].setAlwaysOnTop(true);
    // parametric knobs
    for (int i = 0; i < 4; i++)
    {
        // freq knob
        bandKnobs[i * 3].setBounds(i * 100 + 120, yPosition, bigKnobWidth, bigKnobWidth);
        // gain knob
        bandKnobs[i * 3 + 1].setBounds(i * 100 + 120 + knobOffset, yPosition + knobOffset, smallKnobWidth, smallKnobWidth);
        bandKnobs[i * 3 + 1].setAlwaysOnTop(true);
        // q knob
        bandKnobs[i * 3 + 2].setBounds(i * 100 + 120 + knobOffset, yPosition + 110, smallKnobWidth, smallKnobWidth);
    }
    // buttons
    hpfBypassButton.setBounds(filterKnobs[0].getX(), filterKnobs[0].getY() - 50, smallKnobWidth, 20);
    lpfBypassButton.setBounds(filterKnobs[2].getX(), filterKnobs[2].getY() - 50, smallKnobWidth, 20);
    band1BellButton.setBounds(bandKnobs[1].getX(), bandKnobs[2].getBottom() + 20, smallKnobWidth, 20);
    band4BellButton.setBounds(bandKnobs[10].getX(), bandKnobs[11].getBottom() + 20, smallKnobWidth, 20);
}

void EeqAudioProcessorEditor::drawKnobDiagram(juce::Graphics& g, juce::String textLeft, juce::String textRight, int xPosition, int yPosition)
{
    // outer ring
    g.setColour(juce::Colours::darkgrey);
    g.drawEllipse(float(xPosition - 10), float(yPosition - 10), 20.0f, 20.0f, 2.0f);
    g.fillRect(xPosition + 10, yPosition - 1, 10, 2);
    g.setFont(12.0f);
    g.drawText(textRight, xPosition + 25, yPosition - 6, 28, 12, juce::Justification::centredLeft, false);
    // break in ring
    g.setColour(juce::Colour(0xff242424));
    g.fillRect(xPosition - 12, yPosition - 3, 6, 6);
    // inner circle
    g.setColour(juce::Colours::grey);
    g.fillEllipse(float(xPosition - 5), float(yPosition - 5), 10.0f, 10.0f);
    g.fillRect(xPosition - 20, yPosition - 1, 15, 2);
    g.drawText(textLeft, xPosition - 53, yPosition - 6, 28, 12, juce::Justification::centredRight, false);
}
