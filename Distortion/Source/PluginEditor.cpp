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
    addAndMakeVisible(offsetKnob);
    addAndMakeVisible(hpfKnob);
    addAndMakeVisible(lpfKnob);
    addAndMakeVisible(shapeKnob);
    addAndMakeVisible(shapeButton);
    driveAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "drive", driveKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "volume", volumeKnob);
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "mix", mixKnob);
    angerAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "anger", angerKnob);
    offsetAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "offset", offsetKnob);
    hpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "hpf", hpfKnob);
    lpfAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "lpf", lpfKnob);
    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "shape", shapeKnob);
    shapeButtonAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "shapeTilt", shapeButton);
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
    const int index = static_cast<int>(audioProcessor.parameters.getRawParameterValue("type")->load());
    typeButtons[index].setToggleState(true, juce::NotificationType::dontSendNotification);

    setSize(360, 420);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void DistortionAudioProcessorEditor::resized()
{
    bgImage.setBounds(getLocalBounds());
    powerLine.setBounds(0, 10, 260, 50);
    const int knobWidth = 40;
    const int buttonWidth = 50;
    driveKnob.setBounds(120, 95, 120, 145);
    const int leftPosition = driveKnob.getX() - 10 - knobWidth;
    volumeKnob.setBounds(leftPosition, 60, knobWidth, knobWidth + 25);
    mixKnob.setBounds(leftPosition - 40, 130, knobWidth, knobWidth + 25);
    angerKnob.setBounds(leftPosition, 200, knobWidth, knobWidth + 25);
    offsetKnob.setBounds(leftPosition + 60, 250, knobWidth, knobWidth + 25);
    const int rightPosition = driveKnob.getRight() + 10;
    hpfKnob.setBounds(rightPosition, 60, knobWidth, knobWidth + 25);
    lpfKnob.setBounds(rightPosition + 40, 130, knobWidth, knobWidth + 25);
    shapeKnob.setBounds(rightPosition, 200, knobWidth, knobWidth + 25);
    shapeButton.setBounds(rightPosition - 60, 260, knobWidth, 50);
    for (int type = 0; type < typeButtons.size(); type++)
    {
        const int xPos = (type == 2) ? type * (buttonWidth + 14) + 60 : type * (buttonWidth + 13) + 60;
        typeButtons[type].setBounds(xPos, 350, buttonWidth, buttonWidth);
    }
    const int xPos = typeButtons[0].getX() + (typeButtons[0].getWidth() / 2) - 1;
    const int width = typeButtons[3].getX() + (typeButtons[3].getWidth() / 2) - xPos + 1;
    multiLabel.setBounds(xPos, typeButtons[0].getY() - 20, width, 13);
}

void DistortionAudioProcessorEditor::buttonClicked(int index)
{
    const float choice = static_cast<float>(index / 3.0f);
    audioProcessor.parameters.getParameter("type")->setValueNotifyingHost(choice);
}
