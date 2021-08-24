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

#define numOutputs 2

class LevelLabel : public juce::TextButton
{
public:
    class LevelLabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
        {
            g.setFont(xxii.withHeight(14.0f));
            g.setColour(juce::Colours::grey);
            g.drawFittedText(button.getButtonText(), button.getLocalBounds(), 
                juce::Justification::centred, 1);
        }
    private:
        const juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf,
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    LevelLabel()
    {
        setLookAndFeel(&laf);
        onClick = [&]() {
            reset();
        };
    }

    // update level label if new peak found
    void updateValue(float inputLevel)
    {
        if (inputLevel > currentLevelValue)
        {
            float bufferDecibelValue = juce::Decibels::gainToDecibels(inputLevel);
            juce::String labelText = "";
            if (bufferDecibelValue == -100.0f)
            {
                labelText = "-INF";
            }
            else if (bufferDecibelValue > 0.0f)
            {
                labelText = "+" + (juce::String(juce::Decibels::gainToDecibels(inputLevel), 1)) + " dB";
            }
            else
            {
                labelText = (juce::String(juce::Decibels::gainToDecibels(inputLevel), 1)) + " dB";
            }
            setButtonText(labelText);
            currentLevelValue = inputLevel;
        }
    }

    void reset()
    {
        currentLevelValue = -100.0f;
    }

private:
    LevelLabelLookAndFeel laf;
    float currentLevelValue{ -100.0f };
};

class ClipLight : public juce::TextButton
{
public:
    class ClipLightLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool, bool) override
        {
            const auto buttonArea = button.getLocalBounds();
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

    ClipLight(LevelLabel& levelLabel)
    {
        setLookAndFeel(&laf);
        turnOff();
        onClick = [&]() {
            turnOff();
            levelLabel.reset();
        };
    }

    void updateValue(float maxBufferLevel)
    {
        if (maxBufferLevel > 1.0f)
        {
            turnOn();
        }
    }

private:
    void turnOn()
    {
        setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    };

    void turnOff()
    {
        setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    };

    ClipLightLookAndFeel laf;
};

class GainReductionMeter : public juce::Component, public juce::Timer
{
public:
    GainReductionMeter(float& inputL, float& inputR)
    {
        meterOverlay.addColour(0.5f, juce::Colour(0x99ff0000));
        levelLabel.setBounds(levelLabelBounds);
        addAndMakeVisible(levelLabel);
        startTimerHz(30);
        gainReduction = { &inputL, &inputR };
    }

    void drawLabel(juce::Graphics& g)
    {
        g.setFont(xxii.withHeight(14.0f));
        g.setColour(juce::Colours::grey);
        g.drawText("GR", juce::Rectangle<int>(bandBounds[0].getX(), 0, 20, bandBounds[0].getY()),
            juce::Justification::centredTop, false);
    }

    void drawMeterOutline(juce::Graphics& g)
    {
        g.setColour(juce::Colour(0xff303030));
        g.drawRect(bandBounds[0].expanded(1), 1);
        g.drawRect(bandBounds[1].expanded(1), 1);
    }

    void drawScaleMarkings(juce::Graphics& g)
    {
        g.setFont(9.0f);
        for (const int& mark : marksText)
        {
            const float heightPercentage = bandBounds[0].getHeight() * mark * (1.0f / meterBottomLevel);
            const int markYPosition = static_cast<int>(1 + bandBounds[0].getY() + heightPercentage);
            g.setColour(juce::Colours::grey);
            g.drawText(juce::String(mark), bandBounds[1].getRight(), markYPosition - 6,
                20, 10, juce::Justification::centred, false);
            g.drawRect(0, markYPosition - 1, 4, 1);
        }
    }

    void drawMeter(juce::Graphics& g, float level, int channel)
    {
        const auto bounds = bandBounds[channel];
        int currentLevelY = bounds.getY() - static_cast<int>(level * bounds.getHeight() / meterBottomLevel);
        currentLevelY = std::min(std::max(currentLevelY, bounds.getY()), bounds.getBottom());
        // draw background
        g.setColour(juce::Colours::black);
        g.fillRect(juce::Rectangle<int>(
            bounds.getX(), currentLevelY, bounds.getWidth(), bounds.getBottom() - currentLevelY
            ));
        // draw foreground
        g.setGradientFill(meterFgColor);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), currentLevelY - bounds.getY());
        g.setGradientFill(meterOverlay);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), currentLevelY - bounds.getY());
    }

    void paint(juce::Graphics& g) override
    {
        drawLabel(g);
        drawMeterOutline(g);
        drawScaleMarkings(g);
        std::array<float, 2> inputLevel = { *gainReduction[0], *gainReduction[1] };
        const float maxInputLevel = std::max(inputLevel[0], inputLevel[1]);
        levelLabel.updateValue(juce::Decibels::decibelsToGain(maxInputLevel));
        for (int channel = 0; channel < numOutputs; channel++)
        {
            // test gain reduction and meter level for decay purposes
            if (inputLevel[channel] < meterLevel[channel])
            {
                meterLevel[channel] *= (1 - (1.0f / decayRate));
            }
            else
            {
                meterLevel[channel] = inputLevel[channel];
            }
            drawMeter(g, meterLevel[channel], channel);
        }
    }
    void resized() override {}

    void timerCallback() override
    {
        for (const auto& bounds : bandBounds)
            repaint(bounds);
    }

    int getMeterWidth()
    {
        return bandBounds[1].getRight() + 20;
    }

    int getMeterHeight()
    {
        return bandBounds[0].getBottom() + 28;
    }

private:
    const float meterBottomLevel{ -40.f };  // lowest reoslution of meter
    const float decayRate{ 2.0f };
    std::array<float, numOutputs> meterLevel{ meterBottomLevel, meterBottomLevel };
    std::array<float*, numOutputs> gainReduction;
    const std::array<int, 7> marksText{ 0, -6, -12, -18, -24, -30, -36 };
    LevelLabel levelLabel;
    const std::array<juce::Rectangle<int>, numOutputs> bandBounds{
        juce::Rectangle<int>(10, 20, 10, 200),
        juce::Rectangle<int>(21, 20, 10, 200) };
    const juce::Rectangle<int> levelLabelBounds{ 0, bandBounds[0].getBottom() + 5,
        bandBounds[1].getRight() + 10, 20 };
    const juce::ColourGradient meterFgColor{ juce::Colours::yellow,
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getY()).toFloat(),
        juce::Colours::orangered,
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getBottom()).toFloat(), false };
    juce::ColourGradient meterOverlay{ juce::Colour(0x66ff0000),
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getY()).toFloat(),
        juce::Colour(0x66ff0000),
        juce::Point<int>(bandBounds[1].getRight(), bandBounds[0].getY()).toFloat(), false };
    const juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf,
        BinaryData::XXIIAvenRegular_ttfSize) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainReductionMeter)
};

class LevelMeter : public juce::Component, public juce::Timer
{
public:
    LevelMeter(float& inputL, float& inputR)
    {
        meterOverlay.addColour(0.5f, juce::Colour(0x99ff0000));
        levelLabel.setBounds(levelLabelBounds);
        addAndMakeVisible(levelLabel);
        clipLight.setBounds(clipLightBounds);
        addAndMakeVisible(clipLight);
        startTimerHz(30);
        bufferMagnitude = { &inputL, &inputR };
    }

    void drawMeterOutline(juce::Graphics& g)
    {
        g.setColour(juce::Colour(0xff303030));
        g.drawRect(clipLightBounds.expanded(1), 1);
        g.drawRect(bandBounds[0].expanded(1), 1);
        g.drawRect(bandBounds[1].expanded(1), 1);
    }
    
    void drawScaleMarkings(juce::Graphics& g)
    {
        g.setFont(9.0f);
        for (const int& mark : marksText)
        {
            const float heightPercentage = bandBounds[0].getHeight() * mark * (1.0f / meterBottomLevel);
            const int markYPosition = static_cast<int>(1 + bandBounds[0].getY() + heightPercentage);
            g.setColour(juce::Colours::grey);
            g.drawText(juce::String(mark), bandBounds[1].getRight(), markYPosition - 6,
                20, 10, juce::Justification::centred, false);
            g.drawRect(0, markYPosition - 1, 4, 1);
        }
    }

    void drawMeter(juce::Graphics& g, float level, int channel)
    {
        const auto bounds = bandBounds[channel];
        int currentLevelY = bounds.getY() + static_cast<int>(juce::Decibels::gainToDecibels(level)
            * bounds.getHeight() / meterBottomLevel);
        currentLevelY = std::min(std::max(currentLevelY, bounds.getY()), bounds.getBottom());
        // draw background
        g.setColour(juce::Colours::black);
        g.fillRect(bounds.withHeight(currentLevelY - bounds.getY()));
        // draw foreground
        g.setGradientFill(meterFgColor);
        g.fillRect(bounds.getX(), currentLevelY, bounds.getWidth(), 
            bounds.getY() + bounds.getHeight() - currentLevelY);
        g.setGradientFill(meterOverlay);
        g.fillRect(bounds.getX(), currentLevelY, bounds.getWidth(), 
            bounds.getY() + bounds.getHeight() - currentLevelY);
    }

    void paint(juce::Graphics& g) override
    {
        drawMeterOutline(g);
        drawScaleMarkings(g);
        // get magnitude from processor
        std::array<float, numOutputs> inputLevel = { *bufferMagnitude[0], *bufferMagnitude[1] };
        const float maxBufferLevel = std::max(inputLevel[0], inputLevel[1]);
        levelLabel.updateValue(maxBufferLevel);
        clipLight.updateValue(maxBufferLevel);
        for (int channel = 0; channel < numOutputs; channel++)
        {
            // test input level and meter level for decay purposes
            if (inputLevel[channel] < meterLevel[channel])
            {
                meterLevel[channel] *= (1 - (1.0f / decayRate));
            }
            else
            {
                meterLevel[channel] = inputLevel[channel];
            }
            drawMeter(g, meterLevel[channel], channel);
        }
    }

    void resized() override {}

    void timerCallback() override
    {
        for (const auto& bounds : bandBounds)
            repaint(bounds);
    }

    int getMeterWidth()
    {
        return bandBounds[1].getRight() + 20;
    }

    int getMeterHeight()
    {
        return bandBounds[0].getBottom() + 28;
    }

private:
    const float meterBottomLevel{ -60.f };  // lowest reoslution of meter
    const float decayRate{ 5.0f };
    std::array<float, numOutputs> meterLevel{ meterBottomLevel, meterBottomLevel };
    std::array<float*, numOutputs> bufferMagnitude;
    const std::array<int, 8> marksText{ 0, -3, -6, -10, -16, -22, -32, -48 };
    LevelLabel levelLabel;
    ClipLight clipLight{ levelLabel };
    const std::array<juce::Rectangle<int>, numOutputs> bandBounds{
        juce::Rectangle<int>(10, 12, 10, 200),
        juce::Rectangle<int>(21, 12, 10, 200) };
    const juce::Rectangle<int> clipLightBounds{ 10, 1, 21, 10 };
    const juce::Rectangle<int> levelLabelBounds{ 0, bandBounds[0].getBottom() + 5, 
        bandBounds[1].getRight() + 10, 20 };
    const juce::ColourGradient meterFgColor{ juce::Colours::yellow, 
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getY()).toFloat(),
        juce::Colours::orangered, 
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getBottom()).toFloat(), false };
    juce::ColourGradient meterOverlay{ juce::Colour(0x66ff0000), 
        juce::Point<int>(bandBounds[0].getX(), bandBounds[0].getY()).toFloat(),
        juce::Colour(0x66ff0000), 
        juce::Point<int>(bandBounds[1].getRight(), bandBounds[0].getY()).toFloat(), false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};