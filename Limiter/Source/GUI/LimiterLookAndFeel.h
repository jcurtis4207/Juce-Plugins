/*
*   Limiter Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>

class LimiterLookAndFeel : public juce::LookAndFeel_V4
{
public:
	// override slider
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float,
        float, const juce::Slider::SliderStyle, juce::Slider& slider) override
    {
        // disable textbox
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        // calculate track
        float trackWidth = 6.0f;
        juce::Point<int> startPoint(x + width / 2, height + y);
        juce::Point<int> endPoint( startPoint.x, y);
        // draw track
        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint.toFloat());
        backgroundTrack.lineTo(endPoint.toFloat());
        g.setColour(juce::Colour(0xff253035));
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        // calculate thumb
        juce::Point<int> minPoint, maxPoint;
        int kx = x + width / 2;
        int ky = (int)sliderPos;
        minPoint = startPoint;
        maxPoint = { kx, ky };
        int thumbWidth = 40;
        // draw thumb
        g.setColour(juce::Colours::grey);
        auto thumbBounds = juce::Rectangle<int>(thumbWidth, 20).withCentre(maxPoint);
        g.fillRoundedRectangle(thumbBounds.toFloat(), 3.0f);
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(thumbBounds.toFloat(), 3.0f, 1.0f);
        g.drawText(juce::String(slider.getValue(), 1), thumbBounds.toFloat(), juce::Justification::centred, false);
    }

    // override link knob
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float,
        const float, const float, juce::Slider& slider) override
    {
        float xFloat = (float)x;
        float yFloat = (float)y;
        float widthFloat = (float)width;
        float heightFloat = (float)height;
        // disable textbox
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        // fill knob
        g.setColour(juce::Colours::grey);
        g.fillRoundedRectangle(xFloat, yFloat, widthFloat, heightFloat, 3.0f);
        // draw outline
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(xFloat, yFloat, widthFloat, heightFloat, 3.0f, 1.0f);
        // draw triangles
        g.setColour(juce::Colours::white);
        juce::Path triangles;
        triangles.addTriangle(widthFloat / 2 - 1, yFloat + 7, widthFloat / 2 - 1, heightFloat - 7, xFloat + 5, heightFloat / 2);
        triangles.addTriangle(widthFloat / 2 + 1, yFloat + 7, widthFloat / 2 + 1, heightFloat - 7, widthFloat - 5, heightFloat / 2);
        g.fillPath(triangles);
    }

    // override labels
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        // set font
        const juce::Font font(getLabelFont(label).withHeight(12));
        g.setColour(juce::Colours::white);
        g.setFont(font);
        // get text area
        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
        // draw text
        g.drawFittedText(label.getText(), textArea, juce::Justification::centred,
            juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
            label.getMinimumHorizontalScale());
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
        if (button.getToggleState())
        {
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe9e9e9), buttonRectangle.getCentreX(), buttonRectangle.getCentreY(), juce::Colour(0xffa0a0a0), buttonRectangle.getX(), buttonRectangle.getY(), true));
        }
        else
        {
            g.setColour(juce::Colour(0xffe9e9e9));
        }
        g.fillRoundedRectangle(buttonRectangle, 3.0f);
        // draw outline
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(buttonRectangle, 3.0f, 1.0f);
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
