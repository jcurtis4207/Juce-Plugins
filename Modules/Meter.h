/*
*   Level Meter Module
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
* 
*  Stereo level meter that displays peak level and a clip light
*  Clicking the peak level or clip light resets them
* 
*  Implementation Instructions:
*
*   In PluginProcessor.h, public:
*       create variables float bufferMagnitudeL and bufferMagnitudeR
*   In PluginProcessor.cpp, in processBlock, after all processing add lines:
*       bufferMagnitudeL = buffer.getMagnitude(0, 0, buffer.getNumSamples())
*       bufferMagnitudeR = buffer.getMagnitude((totalNumInputChannels > 1) ? 1 : 0, 0, buffer.getNumSamples())
*   In PluginEditor.h, private:
*       create object Meter meter AFTER audioProcessor reference
*   In PluginEditor.cpp, in constructor:
*       meter.setBounds(<x>, <y>, meter.getMeterWidth(), meter.getMeterHeight())
*       addAndMakeVisible(meter)
*	And add to initialization list:
*		meter(audioProcessor.bufferMagnitudeL, audioProcessor.bufferMagnitudeR)
*
*/
#pragma once

#include <JuceHeader.h>

// Look and feel for clip light
class ClipLightLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonClicked) override
    {
        auto buttonArea = button.getLocalBounds();
        juce::Rectangle<int> buttonRectangle = buttonArea;
        // button is completely specified color
        g.setColour(button.findColour(juce::TextButton::buttonColourId));
        g.fillRect(buttonRectangle);
    }
};

// Look and feel for level label/button
class LevelLabelLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonClicked) override
    {
        auto buttonArea = button.getLocalBounds();
        juce::Rectangle<int> buttonRectangle = buttonArea;
        // button background is always transparent
        g.setColour(juce::Colours::transparentBlack);
        g.fillRect(buttonRectangle);
    }
};

//=======================================================
class Meter  : public juce::Component, public juce::Timer
{
public:
    Meter(float& inputL, float& inputR)
    {
        // read in inputs from processor
        bufferMagnitudeL = &inputL;
        bufferMagnitudeR = &inputR;
        // setup level label/button
        levelLabel.setBounds(levelLabelBounds);
        levelLabel.setLookAndFeel(&levelLabelLookAndFeel);
        levelLabel.onClick = [&]() {
            // reset level value
            currentLevelValue = -100.0f; 
        };
        addAndMakeVisible(levelLabel);
        // setup clip light/button
        clipLight.setBounds(clipLightBounds);
        clipLight.setLookAndFeel(&clipLightLookAndFeel);
        clipLight.setColour(juce::TextButton::buttonColourId, clipLightOff);
        clipLight.onClick = [&]() {
            // turn off clip light and reset level value
            clipLight.setColour(juce::TextButton::buttonColourId, clipLightOff);
            currentLevelValue = -100.0f;
        };
        addAndMakeVisible(clipLight);
        // calculate band bounds
        for (int i = 0; i < sizeof(bands) / sizeof(bands[0]); i++)
        {
            bandBounds[i] = (int)round(meterYPosition + meterTotalHeight * (bands[i] * 1.0 / meterBottomLevel));
        }
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
        g.drawLine(meterSplitLine.toFloat());
        // draw scale markings
        g.setFont(markFontHeight);
        for (const int& mark : marksText)
        {
            int markYPosition = (int)round(1 + meterYPosition + meterTotalHeight * (mark * 1.0 / meterBottomLevel));
            g.setColour(markTextColor);
            g.drawText(juce::String(mark), markTextXPosition, (int)(markYPosition - markFontHeight / 2) - 1, 20, 10, juce::Justification::centred, false);
            g.setColour(markTickColor);
            g.drawRect(markTickXPosition, markYPosition - 1, 4, 1);
        }
        // get magnitude from processor
        float bufferLevel[2] = { *bufferMagnitudeL, *bufferMagnitudeR };
        // update level label if new peak found
        float maxBufferLevel = std::max(bufferLevel[0], bufferLevel[1]);
        if (maxBufferLevel > currentLevelValue)
        {
            float bufferDecibelValue = juce::Decibels::gainToDecibels(maxBufferLevel);
            juce::String labelText = "";
            if (bufferDecibelValue == -100.0f)
            {
                labelText = "-INF";
            }
            else if(bufferDecibelValue > 0.0f)
            {
                labelText = "+" + (juce::String(juce::Decibels::gainToDecibels(maxBufferLevel), 1)) + " dB";
            }
            else
            {
                labelText = (juce::String(juce::Decibels::gainToDecibels(maxBufferLevel), 1)) + " dB";
            }
            levelLabel.setButtonText(labelText);
            currentLevelValue = maxBufferLevel;
        }
        // turn on clip light if level goes over 0.0dBFS -> 1.0f gain
        if (maxBufferLevel > 1.0f)
        {
            clipLight.setColour(juce::TextButton::buttonColourId, clipLightOn);
        }
        // for each channel
        for (int channel = 0; channel < 2; channel++) {
            // test buffer level and meter level for decay purposes
            if (bufferLevel[channel] < meterLevel[channel])
            {
                // decay level based on decay rate
                meterLevel[channel] *= (1 - (1.0f / decayRate));
            }
            else
            {
                meterLevel[channel] = bufferLevel[channel];
            }
            // calculate meter height using Decibel scale
            int meterTopPosition = meterYPosition - (int)round(juce::Decibels::gainToDecibels(meterLevel[channel]) * meterTotalHeight / meterBottomLevel * -1);
            // ensure meter stays within bounds
            meterTopPosition = std::min(std::max(meterTopPosition, meterYPosition), meterYPosition + meterTotalHeight);
            // calculate band x position
            int bandXPosition = (channel == 0) ? meterXPosition : meterXPosition + meterWidth + 1;
            // draw meter bands and visible background bands
            if (meterTopPosition > bandBounds[2])   // inside green band
            {
                // draw background
                g.setColour(meterBgColors[0]);
                g.fillRect(bandXPosition, bandBounds[0], meterWidth, bandBounds[1] - bandBounds[0]);
                g.setColour(meterBgColors[1]);
                g.fillRect(bandXPosition, bandBounds[1], meterWidth, bandBounds[2] - bandBounds[1]);
                g.setColour(meterBgColors[2]);
                g.fillRect(bandXPosition, bandBounds[2], meterWidth, bandBounds[3] - bandBounds[2]);
                // draw meter
                g.setColour(meterFgColors[2]);
                g.fillRect(bandXPosition, meterTopPosition, meterWidth, bandBounds[3] - meterTopPosition);
            }
            else if (meterTopPosition <= bandBounds[2] && meterTopPosition > bandBounds[1]) // inside yellow band
            {
                // draw background
                g.setColour(meterBgColors[0]);
                g.fillRect(bandXPosition, bandBounds[0], meterWidth, bandBounds[1] - bandBounds[0]);
                g.setColour(meterBgColors[1]);
                g.fillRect(bandXPosition, bandBounds[1], meterWidth, bandBounds[2] - bandBounds[1]);
                // draw meter
                g.setColour(meterFgColors[2]);
                g.fillRect(bandXPosition, bandBounds[2], meterWidth, bandBounds[3] - bandBounds[2]);
                g.setColour(meterFgColors[1]);
                g.fillRect(bandXPosition, meterTopPosition, meterWidth, bandBounds[2] - meterTopPosition);
            }
            else if (meterTopPosition <= bandBounds[1]) // inside orange band
            {
                // draw background
                g.setColour(meterBgColors[0]);
                g.fillRect(bandXPosition, bandBounds[0], meterWidth, bandBounds[1] - bandBounds[0]);
                // draw meter
                g.setColour(meterFgColors[2]);
                g.fillRect(bandXPosition, bandBounds[2], meterWidth, bandBounds[3] - bandBounds[2]);
                g.setColour(meterFgColors[1]);
                g.fillRect(bandXPosition, bandBounds[1], meterWidth, bandBounds[2] - bandBounds[1]);
                g.setColour(meterFgColors[0]);
                g.fillRect(bandXPosition, meterTopPosition, meterWidth, bandBounds[1] - meterTopPosition);
            }
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
    const float meterBottomLevel{ -60.f };  // lowest reoslution of meter
    float meterLevel[2]{ meterBottomLevel, meterBottomLevel };
    const juce::Colour meterFgColors[3]{ juce::Colour(0xffff831c), juce::Colour(0xffe7d427), juce::Colour(0xff2cc914) };
    const float decayRate{ 5.0f };
    // meter bounds
    const int meterXPosition{ 15 };
    const int meterYPosition{ 12 };
    const int meterWidth{ 10 };
    const int meterTotalWidth{ 1 + meterWidth * 2 };
    const int meterTotalHeight{ 200 };
    // meter markings
    const juce::Colour markTextColor{ juce::Colours::white };
    const juce::Colour markTickColor{ juce::Colours::darkgrey };
    const int markTextXPosition{ meterXPosition + meterTotalWidth };
    const int markTickXPosition{ meterXPosition - 9 };
    const int marksText[8]{ 0, -3, -6, -10, -16, -22, -32, -48 };
    const float markFontHeight{ 9.0f };
    // meter bands
    const int bands[4]{ 0, -6, -16, (int)meterBottomLevel };
    const juce::Colour meterBgColors[3]{ juce::Colour(0xff7a4111), juce::Colour(0xff857a19), juce::Colour(0xff1d4b16) };
    int bandBounds[4];
    // level label/button
    juce::TextButton levelLabel;
    float currentLevelValue{ -100.0f };
    const int levelLabelWidth{ 70 };
    const juce::Rectangle<int> levelLabelBounds{ meterXPosition + meterWidth - (levelLabelWidth / 2), meterYPosition + meterTotalHeight + 3, levelLabelWidth, 20 };
    // clip light/button
    juce::TextButton clipLight;
    const juce::Colour clipLightOn{ juce::Colour(0xffcc0404) };
    const juce::Colour clipLightOff{ juce::Colour(0xff320000) };
    const int clipLightHeight{ 10 };
    const juce::Rectangle<int> clipLightBounds{ meterXPosition, meterYPosition - clipLightHeight, meterTotalWidth, clipLightHeight };
    // meter outline
    const juce::Colour outlineColor{ juce::Colours::darkgrey };
    const juce::Rectangle<int> meterOutlineBounds{ meterXPosition - 1, meterYPosition - clipLightHeight - 1, meterTotalWidth + 2, meterTotalHeight + clipLightHeight + 2 };
    const juce::Line<int> meterSplitLine{ meterXPosition + meterWidth + 1, meterYPosition - clipLightHeight, meterXPosition + meterWidth + 1, meterTotalHeight + clipLightHeight };
    // create look and feel objects
    LevelLabelLookAndFeel levelLabelLookAndFeel;
    ClipLightLookAndFeel clipLightLookAndFeel;
    // pointers for processor magnitude variables
    float* bufferMagnitudeL;
    float* bufferMagnitudeR;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Meter)
};
