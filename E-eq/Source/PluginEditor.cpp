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
        knobLabels[i].setFont(knobLabels[i].getFont().withHeight(12));
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
    setSize(870, 250);
}

EeqAudioProcessorEditor::~EeqAudioProcessorEditor()
{
}

void EeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff303030));
    // draw divider lines
    g.setColour(juce::Colours::grey);
    juce::Path p;
    p.addLineSegment(juce::Line<float>(81, 34, 75, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(75, 40, 75, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(75, 100, 25, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(25, 140, 25, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(25, 200, 31, 206), 1.0f);

    p.addLineSegment(juce::Line<float>(181, 34, 175, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(175, 40, 175, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(175, 100, 125, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(125, 140, 125, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(125, 200, 131, 206), 1.0f);

    p.addLineSegment(juce::Line<float>(281, 34, 275, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(275, 40, 275, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(275, 100, 225, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(225, 140, 225, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(225, 200, 231, 206), 1.0f);

    p.addLineSegment(juce::Line<float>(420, 34, 425, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(431, 34, 425, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(425, 40, 425, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(420, 206, 425, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(431, 206, 425, 200), 1.0f);

    p.addLineSegment(juce::Line<float>(569, 34, 575, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(575, 40, 575, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(575, 100, 625, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(625, 140, 625, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(625, 200, 620, 206), 1.0f);

    p.addLineSegment(juce::Line<float>(669, 34, 675, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(675, 40, 675, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(675, 100, 725, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(725, 140, 725, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(725, 200, 720, 206), 1.0f);

    p.addLineSegment(juce::Line<float>(769, 34, 775, 40), 1.0f);
    p.addLineSegment(juce::Line<float>(775, 40, 775, 100), 1.0f);
    p.addLineSegment(juce::Line<float>(775, 100, 825, 140), 1.0f);
    p.addLineSegment(juce::Line<float>(825, 140, 825, 200), 1.0f);
    p.addLineSegment(juce::Line<float>(825, 200, 820, 206), 1.0f);

    g.fillPath(p);
}

void EeqAudioProcessorEditor::resized()
{
    int knobWidth = 50;
    int topRowYPosition = 50;
    int bottomRowYPosition = 130;
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