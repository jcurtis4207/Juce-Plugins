/*
*   E-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/EqLookAndFeel.h"
#include "GUI/PowerLine.h"

class EeqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EeqAudioProcessorEditor(EeqAudioProcessor&);
    ~EeqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EeqAudioProcessor& audioProcessor;
    // filter sliders
    juce::Slider filterKnobs[4];
    juce::String filterParamIDs[4]{ "hpfFreq", "hpfSlope", "lpfFreq", "lpfSlope" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterKnobsAttach[4];
    // parametric band sliders
    juce::Slider bandKnobs[12];
    juce::String bandSuffixes[3]{ " Hz", " dB", " Q" };
    juce::String bandParamIDSuffixes[3]{ "Freq", "Gain", "Q"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandKnobsAttach[12];
    juce::Colour bandColors[6]{ 
        juce::Colour(0xff242424),   // black 
        juce::Colour(0xff35669e),   // blue
        juce::Colour(0xff346844),   // green
        juce::Colour(0xffa84130),   // red
        juce::Colour(0xffe9e9e9),   // white
        juce::Colour(0xffc9c9c9)    // dark white
    };
    // filter bypass buttons
    juce::TextButton hpfBypassButton{ "Bypass" };
    juce::TextButton lpfBypassButton{ "Bypass" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfBypassAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lpfBypassAttach;
    // shelf/bell buttons
    juce::TextButton band1BellButton{ "Bell" };
    juce::TextButton band4BellButton{ "Bell" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> band1BellAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> band4BellAttach;
    // create look and feel object
    EqLookAndFeel eqLookAndFeel;
    // create powerline object
    PowerLine powerLine;
    // function to draw knob diagrams
    void drawKnobDiagram(juce::Graphics& g, juce::String textLeft, juce::String textRight, int xPosition, int yPosition);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessorEditor)
};