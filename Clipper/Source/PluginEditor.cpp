/*
*   Clipper Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ClipperAudioProcessorEditor::ClipperAudioProcessorEditor(ClipperAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), grMeter(audioProcessor.gainReduction)
{
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ceilingSlider);
    addAndMakeVisible(thresholdLabel);
    addAndMakeVisible(ceilingLabel);
    addAndMakeVisible(linkKnob);
    addAndMakeVisible(grMeter);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "threshold", thresholdSlider);
    ceilingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "ceiling", ceilingSlider);
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
    setSize(240, 330);
}

ClipperAudioProcessorEditor::~ClipperAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void ClipperAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 240, 50);
    const int yPosition = 80;
    const int sliderWidth = 50;
    const int sliderHeight = 225;
    thresholdSlider.setBounds(20, yPosition, sliderWidth, sliderHeight);
    thresholdLabel.setBounds(thresholdSlider.getX() - 10, thresholdSlider.getY() - 20, 70, 20);
    ceilingSlider.setBounds(110, yPosition, sliderWidth, sliderHeight);
    ceilingLabel.setBounds(ceilingSlider.getX() - 10, ceilingSlider.getY() - 20, 70, 20);
    linkKnob.setBounds(78, 130, 24, 44);
    grMeter.setBounds(180, yPosition - 16, grMeter.getMeterWidth(), grMeter.getMeterHeight());
}

void ClipperAudioProcessorEditor::linkValueChanged()
{
    if (linkFlag)
    {
        // get value from knob
        const float trim = static_cast<float>(linkKnob.getValue());
        // add to parameter value and convert to normalized range -40 - 0
        const float newThreshold = (40.0f + thresholdValue + trim) / 40.0f;
        const float newCeiling = (40.f + ceilingValue + trim) / 40.0f;
        // send new parameter values
        audioProcessor.parameters.getParameter("threshold")->setValueNotifyingHost(newThreshold);
        audioProcessor.parameters.getParameter("ceiling")->setValueNotifyingHost(newCeiling);
    }
}