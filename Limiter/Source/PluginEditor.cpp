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
    : AudioProcessorEditor(&p), audioProcessor(p), inputMeter(audioProcessor.bufferMagnitudeIn), outputMeter(audioProcessor.bufferMagnitudeOut)
{
    // threshold
    thresholdSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    thresholdSlider.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(thresholdSlider);
    thresholdAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "threshold", thresholdSlider);
    thresholdLabel.setText("Threshold", juce::NotificationType::dontSendNotification);
    thresholdLabel.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(thresholdLabel);
    // ceiling
    ceilingSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    ceilingSlider.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(ceilingSlider);
    ceilingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "ceiling", ceilingSlider);
    ceilingLabel.setText("Ceiling", juce::NotificationType::dontSendNotification);
    ceilingLabel.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(ceilingLabel);
    // release
    releaseSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    releaseSlider.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(releaseSlider);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "release", releaseSlider);
    releaseLabel.setText("Release", juce::NotificationType::dontSendNotification);
    releaseLabel.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(releaseLabel);
    // link
    linkKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    linkKnob.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(linkKnob);
    linkKnob.setRange(-40.0, 40.0, 0.1);
    linkKnob.addListener(this);
    linkLabel.setText("Link", juce::NotificationType::dontSendNotification);
    linkLabel.setLookAndFeel(&limiterLookAndFeel);
    addAndMakeVisible(linkLabel);
    // when dragging get parameter values before modifying
    linkKnob.onDragStart = [&]() {
        thresholdValue = audioProcessor.parameters.getRawParameterValue("threshold")->load();
        ceilingValue = audioProcessor.parameters.getRawParameterValue("ceiling")->load();
        audioProcessor.parameters.getParameter("threshold")->beginChangeGesture();
        audioProcessor.parameters.getParameter("ceiling")->beginChangeGesture();
        linkFlag = true;
    };
    // after dragging set parameter values before resetting link knob
    linkKnob.onDragEnd = [&]() {
        linkFlag = false;
        audioProcessor.parameters.getParameter("threshold")->endChangeGesture();
        audioProcessor.parameters.getParameter("ceiling")->endChangeGesture();
        linkKnob.setValue(0.0f);
    };
    // meters
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);

    setSize(350, 330);
}

LimiterAudioProcessorEditor::~LimiterAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void LimiterAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw powerlines
    powerLine.drawPowerLine(g, 95.0f, 10.0f, 100.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 80.0f, 30.0f, 4, 0, "Limiter");
}

void LimiterAudioProcessorEditor::resized()
{
    int startXPosition = 20;
    int yPosition = 80;
    int sliderWidth = 50;
    int sliderHeight = 225;
    thresholdSlider.setBounds(startXPosition, yPosition, sliderWidth, sliderHeight);
    thresholdLabel.setBounds(thresholdSlider.getX() - 10, thresholdSlider.getY() - 20, 70, 20);
    ceilingSlider.setBounds(startXPosition + 150, yPosition, sliderWidth, sliderHeight);
    ceilingLabel.setBounds(ceilingSlider.getX() - 10, ceilingSlider.getY() - 20, 70, 20);
    releaseSlider.setBounds(startXPosition + 250, yPosition, sliderWidth, sliderHeight);
    releaseLabel.setBounds(releaseSlider.getX() - 10, releaseSlider.getY() - 20, 70, 20);
    linkKnob.setBounds(startXPosition + 115, 130, 20, 20);
    linkLabel.setBounds(linkKnob.getX() - 25, linkKnob.getY() - 20, 70, 20);
    inputMeter.setBounds(thresholdSlider.getRight() - 17, yPosition, outputMeter.getMeterWidth(), outputMeter.getMeterHeight());
    outputMeter.setBounds(ceilingSlider.getRight() - 17, yPosition, outputMeter.getMeterWidth(), outputMeter.getMeterHeight());
}

void LimiterAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &linkKnob && linkFlag)
    {
        // get value from knob
        float trim = linkKnob.getValue();
        // add to parameter value and convert to normalized range -40 - 0
        float newThreshold = (40.0f + thresholdValue + trim) / 40.0f;
        float newCeiling = (40.f + ceilingValue + trim) / 40.0f;
        // send new parameter values
        audioProcessor.parameters.getParameter("threshold")->setValueNotifyingHost(newThreshold);
        audioProcessor.parameters.getParameter("ceiling")->setValueNotifyingHost(newCeiling);
    }
}