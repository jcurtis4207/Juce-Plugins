/*
*   Level Meter Modules
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*   In PluginProcessor
*       create variables for bufferMagnitude or gainReduction
*       in process block update these variables
* 
*	In PluginEditor initialization list:
*		meter(audioProcessor.bufferMagnitudeL, audioProcessor.bufferMagnitudeR)
*
*/

#pragma once
#include <JuceHeader.h>

class GainReductionMeter : public juce::Component, public juce::Timer
{
public:
    class LevelLabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
        {
            juce::Font font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize));
            g.setFont(font.withHeight(14.0f));
            g.setColour(juce::Colours::grey);
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
                    juce::Justification::centredTop, 2);
        }
    };

    GainReductionMeter(float& inputL, float& inputR)
    {
        // setup meter overlay
        meterOverlay.addColour(0.5f, juce::Colour(0x99ff0000));
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
        gainReductionLeft = nullptr;
        gainReductionRight = nullptr;
    }
    void paint(juce::Graphics& g) override
    {
        // draw label
        g.setFont(xxii.withHeight(14.0f));
        g.setColour(juce::Colours::grey);
        g.drawText("GR", juce::Rectangle<int>(meterXPosition, 0, 20, meterYPosition), juce::Justification::centredTop, false);
        // draw meter outline
        g.setColour(juce::Colour(0xff303030));
        g.drawRect(meterOutlineBounds, 1);
        g.fillRect(meterSplitLine);
        // draw scale markings
        g.setFont(9.0f);
        g.setColour(juce::Colours::grey);
        for (const int& mark : marksText)
        {
            int markYPosition = (int)round(1.0f + meterYPosition + meterTotalHeight * (mark * 1.0f / meterBottomLevel));
            g.drawText(juce::String(mark), markTextXPosition, (int)(markYPosition - 9 / 2) - 2, 20, 10, juce::Justification::centred, false);
        }
        // get gain reduction from processor
        float inputLevel[2] = { *gainReductionLeft, *gainReductionRight };
        // find max of 2 input values
        float maxInputLevel = std::max(inputLevel[0], inputLevel[1]);
        // update level label if new peak found
        if (maxInputLevel > currentLevelValue)
        {
            // convert -0.0 to 0.0
            juce::String labelText = juce::String((maxInputLevel < 0.1f)? 0.0f : maxInputLevel, 1) + " dB";
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
            g.setColour(juce::Colours::black);
            g.fillRect(bandXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
            // draw meter
            g.setGradientFill(meterFgColor);
            g.fillRect(bandXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
            g.setGradientFill(meterOverlay);
            g.fillRect(bandXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
        }
    }
    void resized() override {}
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
    // meter bounds
    const int meterXPosition{ 17 };
    const int meterYPosition{ 20 };
    const int meterWidth{ 10 };
    const int meterTotalWidth{ meterWidth * 2 + 1};
    const int meterTotalHeight{ 200 };
    // meter settings
    const float meterBottomLevel{ -40.f };  // lowest reoslution of meter
    float meterLevel[2]{ meterBottomLevel, meterBottomLevel };
    const float decayRate{ 2.0f };
    const juce::ColourGradient meterFgColor{ juce::Colours::yellow, (float)meterXPosition, (float)meterYPosition, 
        juce::Colours::red, (float)meterXPosition, float(meterYPosition + meterTotalHeight), false };
    juce::ColourGradient meterOverlay{ juce::Colour(0x66ff0000), (float)meterXPosition, (float)meterYPosition,
        juce::Colour(0x66ff0000), float(meterXPosition + meterTotalWidth), (float)meterYPosition, false };
    // meter markings
    const int markTextXPosition{ meterXPosition + meterTotalWidth };
    const int marksText[7]{ 0, -6, -12, -18, -24, -30, -36 };
    // level label
    LevelLabelLookAndFeel levelLabelLookAndFeel;
    juce::TextButton levelLabel;
    float currentLevelValue{ -100.0f };
    const juce::Rectangle<int> levelLabelBounds{ meterXPosition + meterWidth - 35, meterYPosition + meterTotalHeight + 3, 70, 20 };
    juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    // meter outline
    const juce::Rectangle<int> meterOutlineBounds{ meterXPosition - 1, meterYPosition - 1, meterTotalWidth + 2, meterTotalHeight + 2 };
    const juce::Rectangle<int> meterSplitLine{ meterXPosition + meterWidth, meterYPosition, 2, meterTotalHeight };
    // pointers for processor gain reduction variables
    float* gainReductionLeft;
    float* gainReductionRight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainReductionMeter)
};

class Meter : public juce::Component, public juce::Timer
{
public:
    class ClipLightLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool, bool) override
        {
            auto buttonArea = button.getLocalBounds();
            auto lightOnGradient = juce::ColourGradient(juce::Colours::orange, buttonArea.getTopLeft().toFloat(),
                juce::Colours::orange, buttonArea.getBottomRight().toFloat(), false);
            lightOnGradient.addColour(0.5f, juce::Colours::yellow);
            // set buttonColourId to black to set clip light on
            if (button.findColour(juce::TextButton::buttonColourId) == juce::Colours::black)
            {
                g.setGradientFill(lightOnGradient);
                g.fillRect(buttonArea);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0x99ff0000), buttonArea.getCentre().toFloat(),
                    juce::Colour(0x55ff0000), buttonArea.getBottomLeft().toFloat(), true));
                g.fillRect(buttonArea);
            }
            else
            {
                g.setColour(juce::Colours::black);
                g.fillRect(buttonArea);
            }
        }
    };

    class LevelLabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
        {
            juce::Font xxii = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize));
            g.setFont(xxii.withHeight(14.0f));
            g.setColour(juce::Colours::grey);
            // defaults from LookAndFeel_V2
            auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
            auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
            auto fontHeight = juce::roundToInt(xxii.getHeight() * 0.6f);
            auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
            auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
            auto textWidth = button.getWidth() - leftIndent - rightIndent;
            if (textWidth > 0)
                g.drawFittedText(button.getButtonText(),
                    leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                    juce::Justification::centredTop, 2);
        }
    };

    Meter(float& inputL, float& inputR)
    {
        // setup meter overlay
        meterOverlay.addColour(0.5f, juce::Colour(0x99ff0000));
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
        clipLight.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        clipLight.onClick = [&]() {
            // turn off clip light and reset level value
            clipLight.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            currentLevelValue = -100.0f;
        };
        addAndMakeVisible(clipLight);
        // set timer speed
        startTimerHz(fps);
    }
    ~Meter() override 
    {
        bufferMagnitudeL = nullptr;
        bufferMagnitudeR = nullptr;
    }
    void paint(juce::Graphics& g) override
    {
        // draw meter outline
        g.setColour(juce::Colour(0xff303030));
        g.drawRect(meterOutlineBounds, 1);
        g.fillRect(meterSplitLine);
        g.drawRect(clipLightBounds.expanded(1), 1);
        // draw scale markings
        g.setFont(9.0f);
        for (const int& mark : marksText)
        {
            int markYPosition = (int)round(1.0f + meterYPosition + meterTotalHeight * (mark * 1.0f / meterBottomLevel));
            g.setColour(juce::Colours::grey);
            g.drawText(juce::String(mark), markTextXPosition, (int)(markYPosition - 9 / 2) - 2, 20, 10, juce::Justification::centred, false);
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
            else if (bufferDecibelValue > 0.0f)
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
            clipLight.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        }
        // for each channel
        for (int channel = 0; channel < 2; channel++)
        {
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
            // draw meter background
            g.setColour(juce::Colours::black);
            g.fillRect(bandXPosition, meterYPosition, meterWidth, meterTopPosition - meterYPosition);
            // draw meter foreground
            g.setGradientFill(meterFgColor);
            g.fillRect(bandXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
            g.setGradientFill(meterOverlay);
            g.fillRect(bandXPosition, meterTopPosition, meterWidth, meterYPosition + meterTotalHeight - meterTopPosition);
        }
    }
    void resized() override {}
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
    const float decayRate{ 5.0f };
    // meter bounds
    const int meterXPosition{ 15 };
    const int meterYPosition{ 12 };
    const int meterWidth{ 10 };
    const int meterTotalWidth{ 1 + meterWidth * 2 };
    const int meterTotalHeight{ 200 };
    // meter markings
    const int markTextXPosition{ meterXPosition + meterTotalWidth };
    const int markTickXPosition{ meterXPosition - 9 };
    const int marksText[8]{ 0, -3, -6, -10, -16, -22, -32, -48 };
    // meter colors
    const juce::ColourGradient meterFgColor{ juce::Colours::yellow, (float)meterXPosition, (float)meterYPosition,
        juce::Colours::orangered, (float)meterXPosition, float(meterYPosition + meterTotalHeight), false };
    juce::ColourGradient meterOverlay{ juce::Colour(0x66ff0000), (float)meterXPosition, (float)meterYPosition,
        juce::Colour(0x66ff0000), float(meterXPosition + meterTotalWidth), (float)meterYPosition, false };
    // level label/button
    LevelLabelLookAndFeel levelLabelLookAndFeel;
    juce::TextButton levelLabel;
    float currentLevelValue{ -100.0f };
    const int levelLabelWidth{ 70 };
    const juce::Rectangle<int> levelLabelBounds{ meterXPosition + meterWidth - (levelLabelWidth / 2), meterYPosition + meterTotalHeight + 3, levelLabelWidth, 20 };
    // clip light/button
    ClipLightLookAndFeel clipLightLookAndFeel;
    juce::TextButton clipLight;
    const int clipLightHeight{ 10 };
    const juce::Rectangle<int> clipLightBounds{ meterXPosition, meterYPosition - clipLightHeight, meterTotalWidth, clipLightHeight - 1};
    // meter outline
    const juce::Rectangle<int> meterOutlineBounds{ meterXPosition - 1, meterYPosition - clipLightHeight - 1, meterTotalWidth + 2, meterTotalHeight + clipLightHeight + 2 };
    const juce::Rectangle<int> meterSplitLine{ meterXPosition + meterWidth, meterYPosition - clipLightHeight, 2, meterTotalHeight + clipLightHeight };
    // pointers for processor magnitude variables
    float* bufferMagnitudeL;
    float* bufferMagnitudeR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Meter)
};