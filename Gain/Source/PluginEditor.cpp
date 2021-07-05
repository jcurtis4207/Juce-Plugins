/*
*   Gain Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

GainAudioProcessorEditor::GainAudioProcessorEditor(GainAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), meter(audioProcessor.bufferMagnitudeL, audioProcessor.bufferMagnitudeR)
{
    // gain
    gainKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainKnob.setLookAndFeel(&gainLookAndFeel);
    addAndMakeVisible(gainKnob);
    sliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainKnob);
    addAndMakeVisible(gainLabel);
    // phase
    phaseButton.setButtonText(juce::CharPointer_UTF8("\xc3\x98"));  // unicode U+00D8 -> ascii -> hex = \xc3\x98
    phaseButton.setLookAndFeel(&gainLookAndFeel);
   // phaseButton.onClick = [&]() { /* toggle phase */ audioProcessor.phase = (audioProcessor.phase == 0.0f) ? 1.0f : 0.0f; };
    addAndMakeVisible(phaseButton);
    buttonAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "phase", phaseButton);
    addAndMakeVisible(phaseLabel);
    //  meter
    addAndMakeVisible(meter);
    setSize(200, 300);
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // create powerlines
    powerLine.drawPowerLine(g, 75.0f, 10.0f, 105.0f, 25.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 60.0f, 25.0f, 4, 0, "GAIN");
}

void GainAudioProcessorEditor::resized()
{
    gainKnob.setBounds(20, 80, 80, 80);
    gainLabel.setBounds(gainKnob.getX(), gainKnob.getY() - 20, 80, 20);
    phaseButton.setBounds(45, 200, 30, 30);
    phaseLabel.setBounds(phaseButton.getX() - 15, phaseButton.getY() - 20, 60, 20);
    meter.setBounds(120, 50, meter.getMeterWidth(), meter.getMeterHeight());
}
