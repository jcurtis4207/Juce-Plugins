/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Custom look and feel
class GainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // override how rotary sliders look
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        // create variables for calculations
        auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
        auto centerX = (float)x + (float)width * 0.5f;
        auto centerY = (float)y + (float)height * 0.5f;
        auto xPosition = centerX - radius;
        auto yPosition = centerY - radius;
        auto diameter = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // setup textbox
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);      // move text box to bottom, set not readonly, set size
        slider.setTextValueSuffix(" dB");                                       // add 'dB' to slider value

        // set knob fill color
        g.setColour(juce::Colour(0xfff0f0f0));
        g.fillEllipse(xPosition, yPosition, diameter, diameter);

        // set knob outline color
        g.setColour(juce::Colours::black);
        g.drawEllipse(xPosition, yPosition, diameter, diameter, 1.0f);

        // create pointer rectangle
        juce::Path p;
        auto pointerLength = radius * 0.5f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));

        // draw pointer
        g.setColour(juce::Colours::black);
        g.fillPath(p);
    }
    // override how buttons look
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonClicked) override
    {
        // set button behavior to toggle
        button.setClickingTogglesState(true);
        // get dimensions
        auto buttonArea = button.getLocalBounds();
        juce::Rectangle<float> buttonRectangle(buttonArea.getX(), buttonArea.getY(), buttonArea.getWidth(), buttonArea.getHeight());
        // set background color based on toggle state
        if (button.getToggleState())
        {
            g.setColour(juce::Colour(0xff242424));  // if phase is normal set background to 
        }
        else
        {
            g.setColour(juce::Colour(0xff87ceeb));  // if phase is flipped set background to bright blue
        }
        // create circle via rounded rectange
        g.fillRoundedRectangle(buttonRectangle, buttonArea.getWidth() / 2);
    }
    // override how button text looks
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonClicked) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        // set font color based on toggle state
        if (button.getToggleState())
        {
            g.setColour(juce::Colours::white);
        }
        else
        {
            g.setColour(juce::Colours::black);
        }

        // defaults from LookAndFeel_V2
        auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
        auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
        auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
        auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        auto textWidth = button.getWidth() - leftIndent - rightIndent;
        if (textWidth > 0)
            g.drawFittedText(button.getButtonText(),
                leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                juce::Justification::centred, 2);
    }
};

//==============================================================================
class GainAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                public juce::Slider::Listener, 
                                public juce::Button::Listener
{
public:
    GainAudioProcessorEditor(GainAudioProcessor&);
    ~GainAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    //==============================================================================
    // listener for slider
    void sliderValueChanged(juce::Slider* slider) override;
    // listener for button
    void buttonClicked(juce::Button* button) override;
    // attachment for slider to tree state
    std::unique_ptr <juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttach;
    // attachment for button to tree state
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttach;

private:
    // define slider
    juce::Slider gainSlider;
    // define phase button
    juce::TextButton phaseButton;
    // define look and feel object
    GainLookAndFeel gainLookAndFeel;

    // reference to processor block
    GainAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainAudioProcessorEditor)
};
