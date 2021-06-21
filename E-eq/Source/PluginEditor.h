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
#include "EqLookAndFeel.h"
#include "PowerLine.h"

class EeqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    EeqAudioProcessorEditor(EeqAudioProcessor&);
    ~EeqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    EeqAudioProcessor& audioProcessor;
    // labels
    juce::Label knobLabels[14];
    juce::String knobLabelText[14]{ "Hz", "Gain", "Hz", "Gain", "Q", "Hz", "Gain", "Q", "Hz", "Gain", "Hz", "Slope", "Hz", "Slope" };
    // filter sliders
    juce::Slider filterKnobs[4];
    juce::String filterParamIDs[4]{ "hpfFreq", "hpfSlope", "lpfFreq", "lpfSlope" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterKnobsAttach[4];
    // parametric band sliders
    juce::Slider bandKnobs[10];
    juce::String bandSuffixes[10]{ " Hz", " dB", " Hz", " dB", "", " Hz", " dB", "", " Hz", " dB" };
    juce::String bandParamIDs[10]{ "band1Freq", "band1Gain", "band2Freq", "band2Gain", "band2Q", "band3Freq", "band3Gain", "band3Q", "band4Freq", "band4Gain" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandKnobsAttach[10];
    juce::Colour bandColors[5]{ 
        juce::Colour(0xff242424),   // black 
        juce::Colour(0xff35669e),   // blue
        juce::Colour(0xff346844),   // green
        juce::Colour(0xffa84130),   // red
        juce::Colour(0xffe9e9e9)    // white
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EeqAudioProcessorEditor)
};