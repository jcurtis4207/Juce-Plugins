/*
*   Level Meter Module
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*   Modified to be single color, mono meter with no clip light
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
        bufferMagnitude = &input;
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
        // get magnitude from processor
        float bufferLevel = *bufferMagnitude;
        // update level label if new peak found
        if (bufferLevel > currentLevelValue)
        {
            float bufferDecibelValue = juce::Decibels::gainToDecibels(bufferLevel);
            juce::String labelText = "";
            if (bufferDecibelValue == -100.0f)
            {
                labelText = "-INF";
            }
            else if (bufferDecibelValue > 0.0f)
            {
                labelText = "+" + (juce::String(juce::Decibels::gainToDecibels(bufferLevel), 1)) + " dB";
            }
            else
            {
                labelText = (juce::String(juce::Decibels::gainToDecibels(bufferLevel), 1)) + " dB";
            }
            levelLabel.setButtonText(labelText);
            currentLevelValue = bufferLevel;
        }
        // test buffer level and meter level for decay purposes
        if (bufferLevel < meterLevel)
        {
            // decay level based on decay rate
            meterLevel *= (1 - (1.0f / decayRate));
        }
        else
        {
            meterLevel = bufferLevel;
        }
        // calculate meter height using Decibel scale
        int meterTopPosition = meterYPosition - (int)round(juce::Decibels::gainToDecibels(meterLevel) * meterTotalHeight / meterBottomLevel * -1);
        // ensure meter stays within bounds
        meterTopPosition = std::min(std::max(meterTopPosition, meterYPosition), meterYPosition + meterTotalHeight);
        // draw background
        g.setColour(meterBgColor);
        g.fillRect(meterXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
        // draw meter
        g.setColour(meterFgColor);
        g.fillRect(meterXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
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
    const float decayRate{ 5.0f };
    const juce::Colour meterFgColor{ juce::Colours::yellow };
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
    // pointer for processor magnitude variable
    float* bufferMagnitude;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Meter)
};

//=======================================================
class GainReductionMeter : public juce::Component, public juce::Timer
{
public:
    GainReductionMeter(float& inputL, float& inputR)
    {
        // read in inputs from processor
        gainReductionLeft = &inputL;
        gainReductionRight = &inputR;
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
    ~GainReductionMeter() override
    {
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::transparentBlack);
        // draw meter outline
        g.setColour(outlineColor);
        g.drawRect(meterOutlineBounds, 1);
        g.drawLine(meterSplitLine.toFloat());
        // draw scale markings
        g.setFont(markFontHeight);
        for (const int& mark : marksText)
        {
            int markYPosition = (int)round(1 + meterYPosition + meterTotalHeight * (mark * 1.0 / meterBottomLevel));
            g.setColour(markTextColor);
            g.drawText(juce::String(mark), markTextXPosition, (int)(markYPosition - markFontHeight / 2) - 1, 20, 10, juce::Justification::centred, false);
        }
        // get gain reduction from processor and invert
        float inputLevel[2] = { *gainReductionLeft * -1.0f, *gainReductionRight * -1.0f };
        // find max of 2 input values
        float maxInputLevel = std::max(inputLevel[0], inputLevel[1]);
        // update level label if new peak found
        if (maxInputLevel > currentLevelValue)
        {
            float bufferDecibelValue = juce::Decibels::gainToDecibels(maxInputLevel);
            juce::String labelText = juce::String(maxInputLevel, 1) + " dB";
            levelLabel.setButtonText(labelText);
            currentLevelValue = maxInputLevel;
        }
        // for each channel
        for (int channel = 0; channel < 2; channel++)
        {
            // test gain reduction and meter level for decay purposes
            if (inputLevel[channel] < meterLevel[channel])
            {
                // decay level based on decay rate
                meterLevel[channel] *= (1 - (1.0f / decayRate));
            }
            else
            {
                meterLevel[channel] = inputLevel[channel];
            }
            // calculate meter height using Decibel scale
            int meterTopPosition = meterYPosition - (int)round(meterLevel[channel] * meterTotalHeight / meterBottomLevel);
            // ensure meter stays within bounds
            meterTopPosition = std::min(std::max(meterTopPosition, meterYPosition), meterYPosition + meterTotalHeight);
            // calculate band x position
            int bandXPosition = (channel == 0) ? meterXPosition : meterXPosition + meterWidth + 1;
            // draw background
            g.setColour(meterBgColor);
            g.fillRect(bandXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
            // draw meter
            g.setColour(meterFgColor);
            g.fillRect(bandXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
        }
    }
    void resized() override
    {
    }
    // override timer to repaint only meter region
    void timerCallback() override
    {
        repaint(meterXPosition, meterYPosition, meterTotalWidth, meterTotalHeight);
    }
    // get total used width and height for setting component bounds
    int getMeterWidth()
    {
        return meterTotalWidth + meterXPosition + 20;
    }
    int getMeterHeight()
    {
        return meterTotalHeight + meterYPosition + 25;
    }

private:
    const int fps{ 30 };
    // meter settings
    const float meterBottomLevel{ -40.f };  // lowest reoslution of meter
    float meterLevel[2]{ meterBottomLevel, meterBottomLevel };
    const float decayRate{ 2.0f };
    const juce::Colour meterFgColor{ juce::Colours::yellow };
    const juce::Colour meterBgColor{ juce::Colour(0xff121212) };
    // meter bounds
    const int meterXPosition{ 17 };
    const int meterYPosition{ 12 };
    const int meterWidth{ 10 };
    const int meterTotalWidth{ 1 + meterWidth * 2 };
    const int meterTotalHeight{ 200 };
    // meter markings
    const juce::Colour markTextColor{ juce::Colours::white };
    const int markTextXPosition{ meterXPosition + meterTotalWidth };
    const int marksText[7]{ 0, -6, -12, -18, -24, -30, -36 };
    const float markFontHeight{ 9.0f };
    // level label/button
    juce::TextButton levelLabel;
    float currentLevelValue{ -100.0f };
    const int levelLabelWidth{ 70 };
    const juce::Rectangle<int> levelLabelBounds{ meterXPosition + meterWidth - (levelLabelWidth / 2), meterYPosition + meterTotalHeight + 3, levelLabelWidth, 20 };
    // meter outline
    const juce::Colour outlineColor{ juce::Colours::darkgrey };
    const juce::Rectangle<int> meterOutlineBounds{ meterXPosition - 1, meterYPosition - 1, meterTotalWidth + 2, meterTotalHeight + 2 };
    const juce::Line<int> meterSplitLine{ meterXPosition + meterWidth + 1, meterYPosition, meterXPosition + meterWidth + 1, meterYPosition + meterTotalHeight };
    // create look and feel objects
    LevelLabelLookAndFeel levelLabelLookAndFeel;
    // pointers for processor gain reduction variables
    float* gainReductionLeft;
    float* gainReductionRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainReductionMeter)
};
