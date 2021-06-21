/*
*   Tilt-eq Plugin
*
*   Made by Jacob Curtis
*   Using JUCE Framework
*   Tested on Windows 10 using Reaper in VST3 format
*
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PowerLine.h"

class TiltLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ensure popup appears above slider
    int getSliderPopupPlacement(juce::Slider&) override
    {
        return juce::BubbleComponent::above;
    }
    // change popup font size
    juce::Font getSliderPopupFont(juce::Slider&) override
    {
        return juce::Font(12.0f, juce::Font::plain);
    }
    // override how slider popups look
    void drawBubble(juce::Graphics& g, juce::BubbleComponent& comp, const juce::Point<float>& tip, const juce::Rectangle<float>& body) override
    {
        g.setColour(juce::Colours::black);
        g.fillRect(body);
    }
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
        // disable textbox
        slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        // fill knob
        g.setColour(juce::Colour(0xffae3722));
        g.fillEllipse(xPosition, yPosition, diameter, diameter);
        // draw outline
        g.setColour(juce::Colours::white);
        g.drawEllipse(xPosition, yPosition, diameter, diameter, 1.0f);
        // create pointer rectangle
        juce::Path p;
        auto pointerLength = radius * 0.75f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));
        // draw pointer
        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }
};

class TilteqAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TilteqAudioProcessorEditor(TilteqAudioProcessor&);
    ~TilteqAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TilteqAudioProcessor& audioProcessor;
    // create gui components
    juce::Slider freqKnob, gainKnob;
    juce::Label freqLabel, gainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach, gainAttach;
    // create look and feel object
    TiltLookAndFeel tiltLookAndFeel;
    // create powerline object
    PowerLine powerLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TilteqAudioProcessorEditor)
};
