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
    // setup slider
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    gainSlider.setBounds(gainSliderBounds);
    gainSlider.setLookAndFeel(&gainLookAndFeel);
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);
    sliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "gain", gainSlider);
    // setup gain label
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setBounds(gainLabelBounds);
    addAndMakeVisible(gainLabel);
    // setup button
    phaseButton.setButtonText(juce::CharPointer_UTF8("\xc3\x98"));  // unicode U+00D8 -> ascii -> hex = \xc3\x98
    phaseButton.setBounds(phaseButtonBounds);
    phaseButton.setLookAndFeel(&gainLookAndFeel);
    phaseButton.onClick = [&]() { /* toggle phase */ audioProcessor.phase = (audioProcessor.phase == 0.0f) ? 1.0f : 0.0f; };
    addAndMakeVisible(phaseButton);
    buttonAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "phase", phaseButton);
    // setup phase label
    phaseLabel.setJustificationType(juce::Justification::centred);
    phaseLabel.setBounds(phaseLabelBounds);
    addAndMakeVisible(phaseLabel);
    // setup meter
    meter.setBounds(meterBounds);
    addAndMakeVisible(meter);
    // set default plugin window size
    setSize(200, 300);
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121212));
    // create powerlines
    powerLine.drawPowerLine(g, 75.0f, 10.0f, 105.0f, 25.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 60.0f, 25.0f, 4, 0, "GAIN");
}

// listener override for slider
void GainAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // if listener is triggered by gain slider
    if (slider == &gainSlider)
    {
        // tell the processor to set the gain according to the slider value
        audioProcessor.gain = (float)gainSlider.getValue();
    }
}