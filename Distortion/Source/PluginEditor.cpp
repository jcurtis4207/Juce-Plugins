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
    addAndMakeVisible(bgImage);
    addAndMakeVisible(powerLine);
    addAndMakeVisible(multiLabel);
    addAndMakeVisible(driveKnob);
    addAndMakeVisible(volumeKnob);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(angerKnob);
    addAndMakeVisible(hpfKnob);
    addAndMakeVisible(lpfKnob);
    addAndMakeVisible(shapeKnob);
    addAndMakeVisible(shapeButton);
    driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "drive", driveKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "volume", volumeKnob);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "mix", mixKnob);
    angerAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "anger", angerKnob);
    hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "hpf", hpfKnob);
    lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lpf", lpfKnob);
    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "shape", shapeKnob);
    shapeButtonAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "shapeTilt", shapeButton);
    // setup type buttons
    for (int i = 0; i < 4; i++)
    {
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

void DistortionAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 260, 50);
    int knobWidth = 40;
    int buttonWidth = 50;
    int bigKnobWidth = 120;
    driveKnob.setBounds(120, 95, bigKnobWidth, bigKnobWidth + 25);
    int leftPosition = driveKnob.getX() - 10 - knobWidth;
    volumeKnob.setBounds(leftPosition, 60, knobWidth, knobWidth + 25);
    mixKnob.setBounds(leftPosition - 40, 130, knobWidth, knobWidth + 25);
    angerKnob.setBounds(leftPosition, 200, knobWidth, knobWidth + 25);
    int rightPosition = driveKnob.getRight() + 10;
    hpfKnob.setBounds(rightPosition, 60, knobWidth, knobWidth + 25);
    lpfKnob.setBounds(rightPosition + 40, 130, knobWidth, knobWidth + 25);
    shapeKnob.setBounds(rightPosition, 200, knobWidth, knobWidth + 25);
    shapeButton.setBounds(rightPosition + 60, 210, 30, 50);
    for (int i = 0; i < 4; i++)
    {
        int xPos = (i == 2) ? i * (buttonWidth + 14) + 60 : i * (buttonWidth + 13) + 60;
        typeButtons[i].setBounds(xPos, 300, buttonWidth, buttonWidth);
    }
    int xPos = typeButtons[0].getX() + (typeButtons[0].getWidth() / 2) - 1;
    int width = typeButtons[3].getX() + (typeButtons[3].getWidth() / 2) - xPos + 1;
    multiLabel.setBounds(xPos, typeButtons[0].getY() - 25, width, 18);
}

void DistortionAudioProcessorEditor::buttonClicked(int index)
{
    float choice = (float)index / 3.0f;
    audioProcessor.parameters.getParameter("type")->setValueNotifyingHost(choice);
}
