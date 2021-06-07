/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAudioProcessorEditor::GainAudioProcessorEditor (GainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);   // set slider to vertical
    gainSlider.setRange(GAIN_RANGE_LOW, GAIN_RANGE_HIGH, 0.01f);            // set range, and increment is overridden by parameter
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);   // move text box to bottom, set readonly, set size
    gainSlider.addListener(this);                                           // apply listener to this editor
    addAndMakeVisible(gainSlider);
    // attach slider to parameter using via GAIN_ID
    sliderAttach.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(audioProcessor.parameters, GAIN_ID, gainSlider));
    // set default plugin window size
    setSize (200, 300);
}

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
}

void GainAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill background with dark grey (argb)
    g.fillAll(juce::Colour (0xff121212));
}

void GainAudioProcessorEditor::resized()
{
    // set position/size of slider within editor
    gainSlider.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 100, 100, 200);
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