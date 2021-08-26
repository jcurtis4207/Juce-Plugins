/*
*   Limiter Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

LimiterAudioProcessorEditor::LimiterAudioProcessorEditor(LimiterAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), grMeter(audioProcessor.gainReduction)
{
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(grMeter);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ceilingSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdLabel);
    addAndMakeVisible(ceilingLabel);
    addAndMakeVisible(releaseLabel);
    addAndMakeVisible(linkKnob);
    addAndMakeVisible(stereoButton);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "threshold", thresholdSlider);
    ceilingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "ceiling", ceilingSlider);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "release", releaseSlider);
    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "stereo", stereoButton);
    // when dragging get parameter values before modifying
    linkKnob.onDragStart = [&]() {
        thresholdValue = audioProcessor.parameters.getRawParameterValue("threshold")->load();
        ceilingValue = audioProcessor.parameters.getRawParameterValue("ceiling")->load();
        audioProcessor.parameters.getParameter("threshold")->beginChangeGesture();
        audioProcessor.parameters.getParameter("ceiling")->beginChangeGesture();
        linkFlag = true;
    };
    // while dragging change parameter values by link value
    linkKnob.onValueChange = [&]() {
        linkValueChanged();
    };
    // after dragging set parameter values before resetting link knob
    linkKnob.onDragEnd = [&]() {
        linkFlag = false;
        audioProcessor.parameters.getParameter("threshold")->endChangeGesture();
        audioProcessor.parameters.getParameter("ceiling")->endChangeGesture();
        linkKnob.setValue(0.0f);
    };
    setSize(350, 330);
}

LimiterAudioProcessorEditor::~LimiterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void LimiterAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 250, 50);
    const int yPosition = 80;
    const int sliderWidth = 50;
    const int sliderHeight = 225;
    thresholdSlider.setBounds(20, yPosition, sliderWidth, sliderHeight);
    thresholdLabel.setBounds(thresholdSlider.getX() - 10, thresholdSlider.getY() - 20, 70, 20);
    ceilingSlider.setBounds(thresholdSlider.getX() + 90, yPosition, sliderWidth, sliderHeight);
    ceilingLabel.setBounds(ceilingSlider.getX() - 10, ceilingSlider.getY() - 20, 70, 20);
    releaseSlider.setBounds(ceilingSlider.getX() + 75, yPosition, sliderWidth, sliderHeight - 50);
    releaseLabel.setBounds(releaseSlider.getX() - 10, releaseSlider.getY() - 20, 70, 20);
    linkKnob.setBounds(thresholdSlider.getX() + 58, 130, 24, 40);
    stereoButton.setBounds(releaseSlider.getX(), releaseSlider.getBottom() + 9, 50, 50);
    grMeter.setBounds(releaseSlider.getX() + 85, yPosition - 16, 
        grMeter.getMeterWidth(), grMeter.getMeterHeight());
}

void LimiterAudioProcessorEditor::linkValueChanged()
{
    if (linkFlag)
    {
        const float trim = static_cast<float>(linkKnob.getValue());
        // add trim to parameter value and convert to normalized range -40 - 0
        const float newThreshold = (40.0f + thresholdValue + trim) / 40.0f;
        const float newCeiling = (40.f + ceilingValue + trim) / 40.0f;
        // send new parameter values
        audioProcessor.parameters.getParameter("threshold")->setValueNotifyingHost(newThreshold);
        audioProcessor.parameters.getParameter("ceiling")->setValueNotifyingHost(newCeiling);
    }
}