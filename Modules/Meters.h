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

using namespace juce;

class LevelLabel : public TextButton
{
public:
    class LevelLabelLookAndFeel : public LookAndFeel_V4
    {
    public:
        void drawButtonBackground(Graphics&, Button&, const Colour&, bool, bool) override {}
        void drawButtonText(Graphics& g, TextButton& button, bool, bool) override
        {
            g.setFont(xxii.withHeight(14.0f));
            g.setColour(Colours::grey);
            g.drawFittedText(button.getButtonText(), button.getLocalBounds(), 
                Justification::centred, 1);
        }
    private:
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf,
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
            float bufferDecibelValue = Decibels::gainToDecibels(inputLevel);
            String labelText = "";
            if (bufferDecibelValue == -100.0f)
            {
                labelText = "-INF";
            }
            else if (bufferDecibelValue > 0.0f)
            {
                labelText = "+" + (String(Decibels::gainToDecibels(inputLevel), 1)) + " dB";
            }
            else
            {
                labelText = (String(Decibels::gainToDecibels(inputLevel), 1)) + " dB";
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

class ClipLight : public TextButton
{
public:
    class ClipLightLookAndFeel : public LookAndFeel_V4
    {
    public:
        void drawButtonBackground(Graphics& g, Button& button, const Colour&, bool, bool) override
        {
            const auto buttonArea = button.getLocalBounds();
            auto lightOnGradient = ColourGradient(Colours::orange, buttonArea.getTopLeft().toFloat(),
                Colours::orange, buttonArea.getBottomRight().toFloat(), false);
            lightOnGradient.addColour(0.5f, Colours::yellow);
            // set buttonColourId to black to set clip light on
            if (button.findColour(TextButton::buttonColourId) == Colours::black)
            {
                g.setGradientFill(lightOnGradient);
                g.fillRect(buttonArea);
                g.setGradientFill(ColourGradient(Colour(0x99ff0000), buttonArea.getCentre().toFloat(),
                    Colour(0x55ff0000), buttonArea.getBottomLeft().toFloat(), true));
                g.fillRect(buttonArea);
            }
            else
            {
                g.setColour(Colours::black);
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
        setColour(TextButton::buttonColourId, Colours::black);
    };

    void turnOff()
    {
        setColour(TextButton::buttonColourId, Colours::white);
    };

    ClipLightLookAndFeel laf;
};

class MeterBand : public Component, public Timer
{
public:
    MeterBand()
    {
        startTimerHz(30);
    }

    void setupBand(float& _inputLevel, bool _drawFromTop, float _resolution, 
        float _decayRate, int _width, int _height, int _channel)
    {
        inputLevel = &_inputLevel;
        drawFromTop = _drawFromTop;
        resolution = _resolution;
        decayRate = _decayRate;
        bounds = Rectangle<int>(0, 0, _width, _height);
        meterFgColor = ColourGradient(Colours::yellow, 0, 0, 
            Colours::orangered, 0, static_cast<float>(_height), false);
        meterOverlay = (_channel == 0) ?
            ColourGradient(Colour(0x66ff0000), 0, 0, Colour(0x99ff0000), static_cast<float>(_width), 0, false) :
            ColourGradient(Colour(0x66ff0000), static_cast<float>(_width), 0, Colour(0x99ff0000), 0, 0, false);
    }

    void drawMeterFromTop(Graphics& g, float level)
    {
        int currentLevelY = bounds.getY() - static_cast<int>(level * bounds.getHeight() / resolution);
        currentLevelY = std::min(std::max(currentLevelY, static_cast<int>(bounds.getY())), 
            static_cast<int>(bounds.getBottom()));
        // draw background
        g.setColour(Colours::black);
        g.fillRect(Rectangle<int>(
            bounds.getX(), currentLevelY, bounds.getWidth(), bounds.getBottom() - currentLevelY
            ));
        // draw foreground
        g.setGradientFill(meterFgColor);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), currentLevelY - bounds.getY());
        g.setGradientFill(meterOverlay);
        g.fillRect(bounds.getX(), bounds.getY(), bounds.getWidth(), currentLevelY - bounds.getY());
    }

    void drawMeterFromBottom(Graphics& g, float level)
    {
        int currentLevelY = bounds.getY() + static_cast<int>(Decibels::gainToDecibels(level)
            * bounds.getHeight() / resolution);
        currentLevelY = std::min(std::max(currentLevelY, static_cast<int>(bounds.getY())), 
            static_cast<int>(bounds.getBottom()));
        // draw background
        g.setColour(Colours::black);
        g.fillRect(bounds.withHeight(currentLevelY - bounds.getY()));
        // draw foreground
        g.setGradientFill(meterFgColor);
        g.fillRect(bounds.getX(), currentLevelY, bounds.getWidth(),
            bounds.getY() + bounds.getHeight() - currentLevelY);
        g.setGradientFill(meterOverlay);
        g.fillRect(bounds.getX(), currentLevelY, bounds.getWidth(),
            bounds.getY() + bounds.getHeight() - currentLevelY);
    }

    void paint(Graphics& g) override
    {
        float decayedLevel = meterLevel * (1 - (1.0f / decayRate));
        meterLevel = jmax(decayedLevel, *inputLevel);
        if (drawFromTop)
        {
            drawMeterFromTop(g, meterLevel);
        }
        else
        {
            drawMeterFromBottom(g, meterLevel);
        }
    }

    void resized() override {}

    void timerCallback() override
    {
        repaint();
    }

private:
    bool drawFromTop;
    float resolution, decayRate, meterLevel;
    float* inputLevel;
    Rectangle<int> bounds;
    ColourGradient meterFgColor, meterOverlay;
};

class GainReductionMeter : public Component
{
public:
    GainReductionMeter(std::array<float, 2>& input)
    {
        gainReduction = { &input[0], &input[1] };
        levelLabel.setBounds(levelLabelBounds);
        addAndMakeVisible(levelLabel);
        for (int channel = 0; channel < numOutputs; channel++)
        {
            meterBands[channel].setupBand(input[channel], true, lowestResolution, 2.0f, 
                bandBounds[channel].getWidth(), bandBounds[channel].getHeight(), channel);
            addAndMakeVisible(meterBands[channel]);
            meterBands[channel].setBounds(bandBounds[channel]);
        }
    }

    void drawLabel(Graphics& g)
    {
        g.setFont(xxii.withHeight(14.0f));
        g.setColour(Colours::grey);
        g.drawText("GR", Rectangle<int>(bandBounds[0].getX(), 0, 20, bandBounds[0].getY()),
            Justification::centredTop, false);
    }

    void drawMeterOutline(Graphics& g)
    {
        g.setColour(Colour(0xff303030));
        g.drawRect(bandBounds[0].expanded(1), 1);
        g.drawRect(bandBounds[1].expanded(1), 1);
    }

    void drawScaleMarkings(Graphics& g)
    {
        g.setFont(9.0f);
        for (const int& mark : marksText)
        {
            const float heightPercentage = bandBounds[0].getHeight() * mark * (1.0f / lowestResolution);
            const int markYPosition = static_cast<int>(1 + bandBounds[0].getY() + heightPercentage);
            g.setColour(Colours::grey);
            g.drawText(String(mark), bandBounds[1].getRight(), markYPosition - 6,
                20, 10, Justification::centred, false);
            g.drawRect(0, markYPosition - 1, 4, 1);
        }
    }

    void paint(Graphics& g) override
    {
        drawLabel(g);
        drawMeterOutline(g);
        drawScaleMarkings(g);
        const float maxInputLevel = std::max(*gainReduction[0], *gainReduction[1]);
        levelLabel.updateValue(Decibels::decibelsToGain(maxInputLevel));
    }
    void resized() override {}

    int getMeterWidth()
    {
        return bandBounds[1].getRight() + 20;
    }

    int getMeterHeight()
    {
        return bandBounds[0].getBottom() + 28;
    }

private:
    const float lowestResolution{ -40.f };
    std::array<float*, numOutputs> gainReduction;
    LevelLabel levelLabel;
    std::array<MeterBand, numOutputs> meterBands;
    const std::array<Rectangle<int>, numOutputs> bandBounds{
        Rectangle<int>(10, 20, 10, 200),
        Rectangle<int>(21, 20, 10, 200) };
    const Rectangle<int> levelLabelBounds{ 0, bandBounds[0].getBottom() + 5,
        bandBounds[1].getRight() + 10, 20 };
    const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf,
        BinaryData::XXIIAvenRegular_ttfSize) };
    const std::array<int, 7> marksText{ 0, -6, -12, -18, -24, -30, -36 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainReductionMeter)
};

class LevelMeter : public Component
{
public:
    LevelMeter(std::array<float, 2>& input)
    {
        bufferMagnitude = { &input[0], &input[1] };
        levelLabel.setBounds(levelLabelBounds);
        addAndMakeVisible(levelLabel);
        clipLight.setBounds(clipLightBounds);
        addAndMakeVisible(clipLight);
        for (int channel = 0; channel < numOutputs; channel++)
        {
            meterBands[channel].setupBand(input[channel], false, lowestResolution, 5.0f,
                bandBounds[channel].getWidth(), bandBounds[channel].getHeight(), channel);
            addAndMakeVisible(meterBands[channel]);
            meterBands[channel].setBounds(bandBounds[channel]);
        }
    }

    void drawMeterOutline(Graphics& g)
    {
        g.setColour(Colour(0xff303030));
        g.drawRect(clipLightBounds.expanded(1), 1);
        g.drawRect(bandBounds[0].expanded(1), 1);
        g.drawRect(bandBounds[1].expanded(1), 1);
    }
    
    void drawScaleMarkings(Graphics& g)
    {
        g.setFont(9.0f);
        for (const int& mark : marksText)
        {
            const float heightPercentage = bandBounds[0].getHeight() * mark * (1.0f / lowestResolution);
            const int markYPosition = static_cast<int>(1 + bandBounds[0].getY() + heightPercentage);
            g.setColour(Colours::grey);
            g.drawText(String(mark), bandBounds[1].getRight(), markYPosition - 6,
                20, 10, Justification::centred, false);
            g.drawRect(0, markYPosition - 1, 4, 1);
        }
    }

    void paint(Graphics& g) override
    {
        drawMeterOutline(g);
        drawScaleMarkings(g);
        const float maxBufferLevel = std::max(*bufferMagnitude[0], *bufferMagnitude[1]);
        levelLabel.updateValue(maxBufferLevel);
        clipLight.updateValue(maxBufferLevel);
    }

    void resized() override {}

    int getMeterWidth()
    {
        return bandBounds[1].getRight() + 20;
    }

    int getMeterHeight()
    {
        return bandBounds[0].getBottom() + 28;
    }

private:
    const float lowestResolution{ -60.f };
    std::array<float*, numOutputs> bufferMagnitude;
    std::array<MeterBand, numOutputs> meterBands;
    LevelLabel levelLabel;
    ClipLight clipLight{ levelLabel };
    const std::array<Rectangle<int>, numOutputs> bandBounds{
        Rectangle<int>(10, 12, 10, 200),
        Rectangle<int>(21, 12, 10, 200) };
    const Rectangle<int> clipLightBounds{ 10, 1, 21, 10 };
    const Rectangle<int> levelLabelBounds{ 0, bandBounds[0].getBottom() + 5, 
        bandBounds[1].getRight() + 10, 20 };
    const std::array<int, 8> marksText{ 0, -3, -6, -10, -16, -22, -32, -48 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};