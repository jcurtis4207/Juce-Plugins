/*
*   Distortion Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

DistortionAudioProcessorEditor::DistortionAudioProcessorEditor(DistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // drive
    driveKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    driveKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(driveKnob);
    driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "drive", driveKnob);
    // volume
    volumeKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    volumeKnob.setTextValueSuffix(" dB");
    volumeKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "volume", volumeKnob);
    // mix
    mixKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixKnob.setTextValueSuffix(" %");
    mixKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(mixKnob);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixKnob);
    // anger
    angerKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    angerKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(angerKnob);
    angerAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "anger", angerKnob);
    // hpf
    hpfKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    hpfKnob.setTextValueSuffix(" Hz");
    hpfKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(hpfKnob);
    hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "hpf", hpfKnob);
    // lpf
    lpfKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    lpfKnob.setTextValueSuffix(" Hz");
    lpfKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(lpfKnob);
    lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lpf", lpfKnob);
    // shape
    shapeKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    shapeKnob.setTextValueSuffix(" dB");
    shapeKnob.setLookAndFeel(&distortionLookAndFeel);
    addAndMakeVisible(shapeKnob);
    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shape", shapeKnob);
    shapeButton.setButtonText("Tilt");
    shapeButton.setClickingTogglesState(true);
    addAndMakeVisible(shapeButton);
    shapeButtonAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "shapeTilt", shapeButton);
    // type buttons
    for (int i = 0; i < 4; i++)
    {
        typeButtons[i].setButtonText(audioProcessor.distortionTypes[i]);
        typeButtons[i].setClickingTogglesState(true);
        addAndMakeVisible(typeButtons[i]);
        typeButtons[i].setRadioGroupId(1001);
    }
    typeButtons[0].onClick = [&]() { buttonClicked(0); };
    typeButtons[1].onClick = [&]() { buttonClicked(1); };
    typeButtons[2].onClick = [&]() { buttonClicked(2); };
    typeButtons[3].onClick = [&]() { buttonClicked(3); };
    // set button toggle for active index
    int index = static_cast<int>(audioProcessor.parameters.getRawParameterValue("type")->load());
    typeButtons[index].setToggleState(true, juce::NotificationType::dontSendNotification);

    setSize(360, 360);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff242424));
    // draw powerlines
    powerLine.drawPowerLine(g, 117.0f, 10.0f, 110.0f, 30.0f, 8, 0, "Jacob Curtis");
    powerLine.drawPowerLine(g, 10.0f, 10.0f, 100.0f, 30.0f, 4, 0, "Distortion");
}

void DistortionAudioProcessorEditor::resized()
{
    int knobWidth = 50;
    int bigKnobWidth = 120;
    driveKnob.setBounds(120, 95, bigKnobWidth, bigKnobWidth);
    int leftPosition = driveKnob.getX() - 10 - knobWidth;
    volumeKnob.setBounds(leftPosition, 60, knobWidth, knobWidth);
    mixKnob.setBounds(leftPosition - 40, 130, knobWidth, knobWidth);
    angerKnob.setBounds(leftPosition, 200, knobWidth, knobWidth);
    int rightPosition = driveKnob.getRight() + 10;
    hpfKnob.setBounds(rightPosition, 60, knobWidth, knobWidth);
    lpfKnob.setBounds(rightPosition + 40, 130, knobWidth, knobWidth);
    shapeKnob.setBounds(rightPosition, 200, knobWidth, knobWidth);
    shapeButton.setBounds(rightPosition + 60, 215, 40, 20);
    for (int i = 0; i < 4; i++)
    {
        int xPos = (i == 2) ? i * (knobWidth + 14) + 60 : i * (knobWidth + 13) + 60;
        typeButtons[i].setBounds(xPos, 270, knobWidth, knobWidth);
    }
}

void DistortionAudioProcessorEditor::buttonClicked(int index)
{
    float choice = (float)index / 3.0f;
    audioProcessor.parameters.getParameter("type")->setValueNotifyingHost(choice);
}
