/*
*   Gain Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once

#include <JuceHeader.h>

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
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);      // move text box to bottom, set not readonly, set size
        slider.setTextValueSuffix(" dB");                                       // add 'dB' to slider value
        slider.setColour(juce::Slider::textBoxOutlineColourId, findColour(juce::Slider::textBoxBackgroundColourId));    // set textbox outline to background color
        // fill knob
        g.setColour(juce::Colour(0xfff0f0f0));
        g.fillEllipse(xPosition, yPosition, diameter, diameter);
        // draw outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(xPosition, yPosition, diameter, diameter, 1.0f);
        // create pointer rectangle
        juce::Path p;
        auto pointerLength = radius * 0.7f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));
        // draw pointer
        g.setColour(juce::Colours::black);
        g.fillPath(p);
    }
    // override how buttons look
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool, bool) override
    {
        // set button behavior to toggle
        button.setClickingTogglesState(true);
        // get dimensions
        auto buttonArea = button.getLocalBounds();
        juce::Rectangle<float> buttonRectangle = buttonArea.toFloat();
        // set background color based on toggle state
        juce::ColourGradient buttonGradient;
        if (!button.getToggleState())
        {
            // normal phase
            g.setColour(juce::Colour(0xffe9e9e9));
        }
        else
        {
            // inverted phase
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe9e9e9), buttonRectangle.getCentreX(), buttonRectangle.getCentreY(), juce::Colour(0xffa0a0a0), buttonRectangle.getX(), buttonRectangle.getY(), true));
        }
        g.fillEllipse(buttonRectangle);
        // draw outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(buttonRectangle, 1.0f);
    }
    // override how button text looks
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        g.setColour(juce::Colours::black);
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