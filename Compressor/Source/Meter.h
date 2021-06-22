/*
*   Level Meter Module
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*   Modified to be single color, mono gain reduction meter
*
*/
#pragma once

#include <JuceHeader.h>

// Look and feel for level label/button
class LevelLabelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonClicked) override
    {
        auto buttonArea = button.getLocalBounds();
        juce::Rectangle<int> buttonRectangle = buttonArea;
        // button background is always transparent
        g.setFont(11.0f);
        g.setColour(juce::Colours::transparentBlack);
        g.fillRect(buttonRectangle);
    }
};

//=======================================================
class Meter : public juce::Component, public juce::Timer
{
public:
    Meter(float& input)
    {
        // read in input from processor
        gainReductionValue = &input;
        // setup level label/button
        levelLabel.setBounds(levelLabelBounds);
        levelLabel.setLookAndFeel(&levelLabelLookAndFeel);
        levelLabel.onClick = [&]() {
            // reset level value
            currentLevelValue = -100.0f;
        };
        addAndMakeVisible(levelLabel);
        // set timer speed
        startTimerHz(fps);
    }
    ~Meter() override
    {
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::transparentBlack);
        // draw meter outline
        g.setColour(outlineColor);
        g.drawRect(meterOutlineBounds, 1);
        // draw scale markings
        g.setFont(markFontHeight);
        for (const int& mark : marksText)
        {
            int markYPosition = (int)round(1 + meterYPosition + meterTotalHeight * (mark * 1.0 / meterBottomLevel));
            g.setColour(markTextColor);
            g.drawText(juce::String(mark), markTextXPosition, (int)(markYPosition - markFontHeight / 2) - 1, 20, 10, juce::Justification::centred, false);
        }
        // get gain reduction from processor and invert
        float inputLevel = *gainReductionValue * -1.0f;
        // update level label if new peak found
        if (inputLevel > currentLevelValue)
        {
            float bufferDecibelValue = juce::Decibels::gainToDecibels(inputLevel);
            juce::String labelText = juce::String(inputLevel, 1) + " dB";
            levelLabel.setButtonText(labelText);
            currentLevelValue = inputLevel;
        }
        // test gain reduction and meter level for decay purposes
        if (inputLevel < meterLevel)
        {
            // decay level based on decay rate
            meterLevel *= (1 - (1.0f / decayRate));
        }
        else
        {
            meterLevel = inputLevel;
        }
        // calculate meter height using Decibel scale
        int meterTopPosition = meterYPosition - (int)round(meterLevel * meterTotalHeight / meterBottomLevel);
        // ensure meter stays within bounds
        meterTopPosition = std::min(std::max(meterTopPosition, meterYPosition), meterYPosition + meterTotalHeight);
        // draw background
        g.setColour(meterBgColor);
        g.fillRect(meterXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
        // draw meter
        g.setColour(meterFgColor);
        g.fillRect(meterXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
    }
    void resized() override
    {
    }
    // override timer to repaint only meter region
    void timerCallback() override
    {
        repaint(meterXPosition, meterYPosition, meterWidth, meterTotalHeight);
    }
    // get total used width and height for setting component bounds
    int getMeterWidth()
    {
        return meterWidth + meterXPosition + 20;
    }
    int getMeterHeight()
    {
        return meterTotalHeight + meterYPosition + 25;
    }

private:
    const int fps{ 30 };
    // meter settings
    const float meterBottomLevel{ -40.f };  // lowest reoslution of meter
    float meterLevel{ meterBottomLevel };
    const float decayRate{ 2.0f };
    const juce::Colour meterFgColor{ juce::Colour(0xffb01d15) };
    const juce::Colour meterBgColor{ juce::Colour(0xff121212) };
    // meter bounds
    const int meterXPosition{ 17 };
    const int meterYPosition{ 12 };
    const int meterWidth{ 10 };
    const int meterTotalHeight{ 200 };
    // meter markings
    const juce::Colour markTextColor{ juce::Colours::white };
    const int markTextXPosition{ meterXPosition + meterWidth };
    const int marksText[7]{ 0, -6, -12, -18, -24, -30, -36 };
    const float markFontHeight{ 9.0f };
    // level label/button
    juce::TextButton levelLabel;
    float currentLevelValue{ -100.0f };
    const int levelLabelWidth{ 70 };
    const juce::Rectangle<int> levelLabelBounds{ meterXPosition + (meterWidth / 2) - (levelLabelWidth / 2), meterYPosition + meterTotalHeight + 3, levelLabelWidth, 20 };
    // meter outline
    const juce::Colour outlineColor{ juce::Colours::darkgrey };
    const juce::Rectangle<int> meterOutlineBounds{ meterXPosition - 1, meterYPosition - 1, meterWidth + 2, meterTotalHeight + 2 };
    // create look and feel objects
    LevelLabelLookAndFeel levelLabelLookAndFeel;
    // pointer for processor gain reduction variable
    float* gainReductionValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Meter)
};
