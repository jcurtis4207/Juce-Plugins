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
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos,
        float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        // disable textbox
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
        // calculate track
        float trackWidth = 6.0f;
        juce::Point<int> startPoint(x + width * 0.5f, height + y);
        juce::Point<int> endPoint( startPoint.x, y);
        // draw track
        juce::Path backgroundTrack;
        backgroundTrack.startNewSubPath(startPoint.toFloat());
        backgroundTrack.lineTo(endPoint.toFloat());
        g.setColour(juce::Colour(0xff253035));
        g.strokePath(backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        // calculate thumb
        juce::Point<int> minPoint, maxPoint;
        int kx = x + width * 0.5f;
        int ky = sliderPos;
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
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        // disable textbox
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        // fill knob
        g.setColour(juce::Colours::grey);
        g.fillRoundedRectangle(x, y, width, height, 3.0f);
        // draw outline
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(x, y, width, height, 3.0f, 1.0f);
        // draw triangles
        g.setColour(juce::Colours::white);
        juce::Path leftTriangle;
        leftTriangle.startNewSubPath(width / 2 - 1, y + 7);
        leftTriangle.lineTo(width / 2 - 1, height - 7);
        leftTriangle.lineTo(x + 5, height / 2);
        leftTriangle.closeSubPath();
        g.fillPath(leftTriangle);
        juce::Path rightTriangle;
        rightTriangle.startNewSubPath(width / 2 + 1, y + 7);
        rightTriangle.lineTo(width / 2 + 1, height - 7);
        rightTriangle.lineTo(width - 5, height / 2);
        rightTriangle.closeSubPath();
        g.fillPath(rightTriangle);
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
};
