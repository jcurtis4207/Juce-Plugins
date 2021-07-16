/*
*   GUI Components Modules
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*/

#pragma once
#include <JuceHeader.h>

/* SSL-style knob with label underneath */
class SmallKnob : public juce::Slider
{
public:
    class SmallKnobLaF : public juce::LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(juce::Slider&) override
        {
            return juce::BubbleComponent::above;
        }
        juce::Font getSliderPopupFont(juce::Slider&) override
        {
            return juce::Font(12.0f, juce::Font::plain);
        }
        void drawBubble(juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body) override
        {
            g.setColour(juce::Colours::black);
            g.fillRect(body);
        }
        void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
        {
            // disable textbox
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            // get dimensions
            float textHeight = 14.0f;
            float diameter = juce::jmin((float)height - textHeight, (float)width);
            float radius = diameter * 0.5f;
            float centerX = (float)width * 0.5f;
            float centerY = radius;
            float xPosition = centerX - radius;
            float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto circleArea = juce::Rectangle<float>(xPosition, 0.0f, diameter, diameter);
            // set hit box
            SmallKnob* knob = static_cast<SmallKnob*>(&slider);
            knob->setHitArea(juce::Point<float>(centerX, centerY), radius);
			knob = nullptr;
            // create shadow
            juce::Path shadow;
            shadow.addEllipse(circleArea.reduced(4.0f));
            auto dropShadow = juce::DropShadow(juce::Colour(0xff000000), 15, juce::Point<int>(0, 10));
            dropShadow.drawForPath(g, shadow);
            // draw bumps
            float bumpGap = 0.15f;
            auto bumpFill = juce::Rectangle<float>(
                juce::Point<float>(centerX, centerY).getPointOnCircumference(radius, (juce::float_Pi / 3.0f) + bumpGap),
                juce::Point<float>(centerX, centerY).getPointOnCircumference(radius, (2 * juce::float_Pi / 3.0f) - bumpGap).translated(-10.0f, 0.0f));
            juce::Path bumps;
            for (int i = 0; i < 6; i++)
            {
                bumps.addPieSegment(circleArea, bumpGap, (juce::float_Pi / 3.0f) - bumpGap, 0.0f);
                bumps.addRectangle(bumpFill);
                bumps.applyTransform(juce::AffineTransform::rotation(juce::float_Pi / 3.0f, centerX, centerY));
            }
            bumps.applyTransform(juce::AffineTransform::rotation(angle, centerX, centerY));
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb0b0b0), centerX, 0, juce::Colour(0xff303030), centerX, diameter, false));
            g.fillPath(bumps);
            // draw outer circle
            g.drawEllipse(circleArea.reduced(4.0f), 4.0f);
            // draw inner circle
            auto innerGradient = juce::ColourGradient(juce::Colour(0xff303030), centerX, 0, juce::Colour(0xff303030), centerX, diameter, false);
            innerGradient.addColour(0.5f, juce::Colour(0xff202020));
            g.setGradientFill(innerGradient);
            g.fillEllipse(circleArea.reduced(4.0f));
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), centerX, 0, juce::Colour(0xff101010), centerX, diameter, false));
            g.drawEllipse(circleArea.reduced(4.0f), 2.0f);
            // draw pointer
            juce::Path pointer;
            float pointerLength = radius * 0.8f;
            float pointerThickness = 4.0f;
            pointer.addRectangle(centerX - (pointerThickness * 0.5f), 4.0f, pointerThickness, pointerLength);
            pointer.applyTransform(juce::AffineTransform::rotation(angle, centerX, centerY));
            g.setColour(juce::Colour(0xffa0a0a0));
            g.fillPath(pointer);
            // draw label
            g.setColour(juce::Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            g.drawText(slider.getName(), 0, int(diameter + 10), width, (int)textHeight, juce::Justification::centredTop, false);
        }
    private:
        juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    SmallKnob(const juce::String& name="", const juce::String& suffix="")
    {
        setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        setLookAndFeel(&laf);
        setName(name);
        setTextValueSuffix(juce::String(" ")+suffix);
        knobRadius = 0.0f;
        center = juce::Point<float>(0.0f, 0.0f);
        setPaintingIsUnclipped(true);
    }

    ~SmallKnob() {}

    void setHitArea(juce::Point<float> p, float r)
    {
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        auto input = juce::Point<float>((float)x, (float)y);
        auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }
    
private:
    juce::Point<float> center;
    float knobRadius;
    SmallKnobLaF laf;
};

/* Neve-style outer knob for nested knob setup */
class OuterKnob : public juce::Slider
{
public:
    class OuterKnobLaF : public juce::LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(juce::Slider&) override
        {
            return juce::BubbleComponent::above;
        }
        juce::Font getSliderPopupFont(juce::Slider&) override
        {
            return juce::Font(12.0f, juce::Font::plain);
        }
        void drawBubble(juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body) override
        {
            g.setColour(juce::Colours::black);
            g.fillRect(body);
        }
        void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
        {
            // disable textbox
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            // get dimensions
            float textHeight = 14.0f;
            float diameter = juce::jmin((float)height - textHeight, (float)width);
            float radius = diameter * 0.5f;
            float centerX = (float)width * 0.5f;
            float centerY = radius;
            float xPosition = centerX - radius;
            float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto circleArea = juce::Rectangle<float>(xPosition, 0.0f, diameter, diameter);
            // set hit box
            OuterKnob* knob = static_cast<OuterKnob*>(&slider);
            knob->setHitArea(juce::Point<float>(centerX, centerY), radius);
            knob = nullptr;
			// create shadow
            juce::Path shadow;
            shadow.addEllipse(circleArea.reduced(2.0f));
            auto dropShadow = juce::DropShadow(juce::Colour(0xff000000), 10, juce::Point<int>(0, 5));
            dropShadow.drawForPath(g, shadow);
            // draw knob face
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb0b0b0), centerX, 0, juce::Colour(0xff303030), centerX, diameter, false));
            g.fillEllipse(circleArea);
            g.setColour(juce::Colours::black);
            g.drawEllipse(circleArea, 1.0f);
            // draw knob hole
            g.setColour(juce::Colours::black);
            auto hole = juce::Rectangle<float>(circleArea.getCentreX() - 28, circleArea.getCentreY() - 28, 56, 56);
            g.fillEllipse(hole);
            // draw knob hole rim
            hole.expand(1.0f, 1.0f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff303030), hole.getX(), hole.getY(),
                juce::Colour(0xffb0b0b0), hole.getX(), hole.getBottom(), false));
            g.drawEllipse(hole, 2.0f);
            // draw pointer
            juce::Path pointer;
            pointer.addEllipse(centerX - 2.5f, 3.0f, 5.0f, 5.0f);
            pointer.applyTransform(juce::AffineTransform::rotation(angle, centerX, centerY));
            g.setColour(juce::Colour(0xff202020));
            g.fillPath(pointer);
        }
    };

    OuterKnob(const juce::String& suffix="")
    {
        setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        setTextValueSuffix(juce::String(" ") + suffix);
        setLookAndFeel(&laf);
        knobRadius = 0.0f;
        center = juce::Point<float>(0.0f, 0.0f);
    }

    ~OuterKnob() {}

    void setHitArea(juce::Point<float> p, float r)
    {
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        auto input = juce::Point<float>((float)x, (float)y);
        auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }

    // get bounding rectangle for inner knob
    juce::Rectangle<int> getInnerArea()
    {
        auto area = getBounds();
        int xPosition = area.getX() + (area.getWidth() / 2) - 25;
        int yPosition = area.getY() + (area.getWidth() / 2) - 25;
        auto output = juce::Rectangle<int>(xPosition, yPosition, 50, 80);
        return output;
    }

private:
    juce::Point<float> center;
    float knobRadius;
    OuterKnobLaF laf;
};

/* Knob with illuminated ticks and label underneath */
class BigKnob : public juce::Slider
{
public:
    class BigKnobLaF : public juce::LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(juce::Slider&) override
        {
            return juce::BubbleComponent::above;
        }
        juce::Font getSliderPopupFont(juce::Slider&) override
        {
            return juce::Font(12.0f, juce::Font::plain);
        }
        void drawBubble(juce::Graphics& g, juce::BubbleComponent&, const juce::Point<float>&, const juce::Rectangle<float>& body) override
        {
            g.setColour(juce::Colours::black);
            g.fillRect(body);
        }
        void drawRotarySlider(juce::Graphics& g, int, int, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
        {
            // disable textbox
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            // get dimensions
            float textHeight = 14.0f;
            float diameter = juce::jmin((float)height - textHeight, (float)width);
            float radius = diameter * 0.5f;
            float centerX = (float)width * 0.5f;
            float centerY = radius;
            float xPosition = centerX - radius;
            float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto circleArea = juce::Rectangle<float>(xPosition, 0.0f, diameter, diameter);
            // calculate marks
            float markAngle = (rotaryEndAngle - rotaryStartAngle) / 12;
            int markBin = int((angle - rotaryStartAngle) / ((rotaryEndAngle - rotaryStartAngle) / 12.0f));
            juce::Path activeMarks, inactiveMarks;
            for (int i = 12; i >= 0; i--)
            {
                if (i > markBin)
                    inactiveMarks.addRoundedRectangle(centerX - 1.0f, 0.0f, 2.0f, 8.0f, 1.0f);
                else
                    activeMarks.addRoundedRectangle(centerX - 1.0f, 0.0f, 2.0f, 8.0f, 1.0f);
                activeMarks.applyTransform(juce::AffineTransform::rotation(markAngle, centerX, centerY));
                inactiveMarks.applyTransform(juce::AffineTransform::rotation(markAngle, centerX, centerY));
            }
            activeMarks.applyTransform(juce::AffineTransform::rotation(rotaryStartAngle - ((rotaryEndAngle - rotaryStartAngle) / 12), centerX, centerY));
            inactiveMarks.applyTransform(juce::AffineTransform::rotation(rotaryStartAngle - ((rotaryEndAngle - rotaryStartAngle) / 12), centerX, centerY));
            // fill marks
            g.setColour(juce::Colour(0xff404040));
            g.fillPath(inactiveMarks);
            g.setColour(juce::Colours::yellow);
            g.fillPath(activeMarks);
            auto glow = juce::DropShadow(juce::Colour(0x99ff0000), 20, juce::Point<int>(0, 0));
            glow.drawForPath(g, activeMarks);
            // set hit box
            circleArea.reduce(15.0f, 15.0f);
            BigKnob* bigKnob = static_cast<BigKnob*>(&slider);
            bigKnob->setHitArea(juce::Point<float>(centerX, centerY), circleArea.getWidth() * 0.5f);
            bigKnob = nullptr;
			// create shadow
            juce::Path shadow;
            shadow.addEllipse(circleArea);
            auto dropShadow = juce::DropShadow(juce::Colour(0xff000000), 15, juce::Point<int>(0, 10));
            dropShadow.drawForPath(g, shadow);
            // draw inner circle
            auto innerGradient = juce::ColourGradient(juce::Colour(0xff303030), centerX, circleArea.getY(),
                juce::Colour(0xff303030), centerX, circleArea.getBottom(), false);
            innerGradient.addColour(0.5f, juce::Colour(0xff202020));
            g.setGradientFill(innerGradient);
            g.fillEllipse(circleArea);
            // draw rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), centerX, circleArea.getY(),
                juce::Colour(0xff000000), centerX, circleArea.getBottom(), false));
            g.drawEllipse(circleArea.reduced(1.0f), 2.0f);
            // draw pointer
            juce::Path pointer;
            pointer.addTriangle(centerX, 22.0f, centerX - 4.0f, 30.0f, centerX + 4.0f, 30.0f);
            pointer.applyTransform(juce::AffineTransform::rotation(angle, centerX, centerY));
            auto pointerArea = pointer.getBounds();
            g.setGradientFill(juce::ColourGradient(juce::Colours::yellow, pointerArea.getCentreX(), pointerArea.getCentreY(),
                juce::Colours::orange, pointerArea.getX(), pointerArea.getY(), true));
            g.fillPath(pointer);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff101010), pointerArea.getCentreX(), pointerArea.getY(),
                juce::Colour(0xff505050), pointerArea.getCentreX(), pointerArea.getBottom(), false));
            g.strokePath(pointer, { 1.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt });
            glow.drawForPath(g, pointer);
            // draw label
            g.setColour(juce::Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            g.drawText(slider.getName(), 0, int(diameter + 10), width, (int)textHeight, juce::Justification::centredTop, false);
        }
    private:
        juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    BigKnob(const juce::String& name="", const juce::String& suffix="")
    {
        setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        setTextValueSuffix(juce::String(" ") + suffix);
        setLookAndFeel(&laf);
        setName(name);
        knobRadius = 0.0f;
        center = juce::Point<float>(0.0f, 0.0f);
        setPaintingIsUnclipped(true);
    }

    ~BigKnob() {}

    void setHitArea(juce::Point<float> p, float r)
    {
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        auto input = juce::Point<float>((float)x, (float)y);
        auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }

private:
    juce::Point<float> center;
    float knobRadius;
    BigKnobLaF laf;
};

/* Slider with value on the thumb */
class VerticalSlider : public juce::Slider
{
public:
    class VerticalSliderLaF : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float,
            float, const juce::Slider::SliderStyle, juce::Slider& slider) override
        {
            // disable textbox
            slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            // calculate track
            float trackWidth = 6.0f;
            auto trackRectangle = juce::Rectangle<float>(float(x + width * 0.5f) - (trackWidth * 0.5f), (float)y, trackWidth, float(height + y - 5));
            // draw track
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(trackRectangle, 2.0f);
            g.setColour(juce::Colour(0xff303030));
            g.drawRoundedRectangle(trackRectangle, 2.0f, 1.0f);
            // calculate thumb
            auto maxPoint = juce::Point<int>((int)trackRectangle.getCentreX(), (int)sliderPos);
            auto thumbBounds = juce::Rectangle<int>(40, 20).withCentre(maxPoint);
            // set hit box
            VerticalSlider* verticalSlider = static_cast<VerticalSlider*>(&slider);
            verticalSlider->setHitArea(maxPoint);
            verticalSlider = nullptr;
            // draw shadow
            auto dropShadow = juce::DropShadow(juce::Colour(0xdd000000), 10, juce::Point<int>(0, 5));
            dropShadow.drawForRectangle(g, thumbBounds.reduced(2));
            // draw thumb
            auto thumbGradient = juce::ColourGradient(juce::Colour(0xff303030), (float)thumbBounds.getX(), (float)thumbBounds.getY(),
                juce::Colour(0xff303030), (float)thumbBounds.getX(), (float)thumbBounds.getBottom(), false);
            thumbGradient.addColour(0.5f, juce::Colour(0xff272727));
            g.setGradientFill(thumbGradient);
            g.fillRoundedRectangle(thumbBounds.toFloat(), 2.0f);
            // draw rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), (float)thumbBounds.getX(), (float)thumbBounds.getY(),
                juce::Colour(0xff101010), (float)thumbBounds.getX(), (float)thumbBounds.getBottom(), false));
            g.drawRoundedRectangle(thumbBounds.toFloat(), 2.0f, 1.0f);
            // draw value
            g.setColour(juce::Colours::grey);
            g.setFont(xxii.withHeight(16.0f));
            g.drawText(juce::String(slider.getValue(), 1), thumbBounds.translated(0, -1).toFloat(), juce::Justification::centred, false);
        }
    private:
        juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    VerticalSlider(const juce::String& suffix="")
    {
        setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        setTextValueSuffix(juce::String(" ") + suffix);
        setLookAndFeel(&laf);
    }
    ~VerticalSlider(){}

    void setHitArea(juce::Point<int> p)
    {
        center = p;
    }

    bool hitTest(int x, int y) override
    {
        auto area = juce::Rectangle<int>(40, 20).withCentre(center);
        auto point = juce::Point<int>(x, y);
        return(area.contains(point));
    }

private:
    juce::Point<int> center;
    VerticalSliderLaF laf;
};

/* Immovable knob with label above */
class LinkKnob : public juce::Slider
{
public:
    class LinkKnobLaF : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float,
            const float, const float, juce::Slider& slider) override
        {
            float textHeight = 14.0f;
            float diameter = juce::jmin((float)width, float(height - textHeight)) - 4.0f;
            auto boxArea = juce::Rectangle<float>(float(width / 2 + x) - (diameter * 0.5f), float(y + textHeight), diameter, diameter);
            // set hit box
            LinkKnob* linkKnob = static_cast<LinkKnob*>(&slider);
            linkKnob->setHitArea(boxArea);
			linkKnob = nullptr;
            // disable textbox
            slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
            // draw outline
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(boxArea.expanded(2.0f), 3.0f);
            // draw knob
            auto innerGradient = juce::ColourGradient(juce::Colour(0xff303030), boxArea.getX(), boxArea.getY(),
                juce::Colour(0xff303030), boxArea.getX(), boxArea.getBottom(), false);
            innerGradient.addColour(0.5f, juce::Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(boxArea, 3.0f);
            // draw rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), boxArea.getX(), boxArea.getY(),
                juce::Colour(0xff101010), boxArea.getX(), boxArea.getBottom(), false));
            g.drawRoundedRectangle(boxArea, 3.0f, 1.0f);
            // draw triangles
            g.setColour(juce::Colours::grey);
            juce::Path triangles;
            float centerX = boxArea.getWidth() * 0.5f + boxArea.getX();
            float centerY = boxArea.getHeight() * 0.5f + boxArea.getY();
            triangles.addTriangle(centerX - 1, boxArea.getY() + 7, centerX - 1, boxArea.getBottom() - 7, boxArea.getX() + 5, centerY);
            triangles.addTriangle(centerX + 1, boxArea.getY() + 7, centerX + 1, boxArea.getBottom() - 7, boxArea.getRight() - 5, centerY);
            g.fillPath(triangles);
            // draw label
            g.setColour(juce::Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            auto textArea = juce::Rectangle<int>(x, y, width, (int)textHeight);
            g.drawText("Link", textArea.toFloat(), juce::Justification::centred, false);
        }
    private:
        juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    LinkKnob()
    {
        setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
        setLookAndFeel(&laf);
        setRange(-40.0, 40.0, 0.1);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    ~LinkKnob() {}

    void setHitArea(juce::Rectangle<float> r)
    {
        area = r;
    }

    bool hitTest(int x, int y)
    {
        auto point = juce::Point<int>(x, y);
        return(area.contains(point.toFloat()));
    }

private:
    juce::Rectangle<float> area;
    LinkKnobLaF laf;
};

/* Horizontal button with light bar */
class SmallButton : public juce::TextButton
{
public:
    class SmallButtonLaF : public juce::LookAndFeel_V4
    {
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool, bool) override
        {
            button.setClickingTogglesState(true);
            // get dimensions
            auto area = button.getLocalBounds().toFloat();
            float width = area.getWidth() - 4.0f;
            float height = 20.0f;
            float xPosition = area.getX() + 2.0f;
            float yPosition = area.getY() + 2.0f;
            float centerX = width * 0.5f + xPosition;
            float centerY = height * 0.5f + yPosition;
            auto buttonArea = juce::Rectangle<float>(xPosition, yPosition, width, height);
            // set hit box
            SmallButton* smallButton = static_cast<SmallButton*>(&button);
            smallButton->setHitArea(buttonArea);
			smallButton = nullptr;
            // draw outline
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(buttonArea.expanded(2.0f), 3.0f);
            // draw button face
            auto innerGradient = juce::ColourGradient(juce::Colour(0xff303030), centerX, buttonArea.getY(),
                juce::Colour(0xff303030), centerX, buttonArea.getBottom(), false);
            innerGradient.addColour(0.5f, juce::Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(buttonArea, 2.0f);
            // draw rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), centerX, buttonArea.getY(),
                juce::Colour(0xff101010), centerX, buttonArea.getBottom(), false));
            g.drawRoundedRectangle(buttonArea, 2.0f, 1.0f);
            // create light
            auto glow = juce::DropShadow(juce::Colour(0x66ff0000), 14, juce::Point<int>(0, -5));
            auto line = juce::Rectangle<float>(xPosition + 5.0f, centerY - 1.5f, width - 10.0f, 3.0f);
            auto lightOnGradient = juce::ColourGradient(juce::Colours::orange, line.getX(), centerY, juce::Colours::orange, line.getRight(), centerY, false);
            lightOnGradient.addColour(0.5f, juce::Colours::yellow);
            auto lightOffGradient = juce::ColourGradient(juce::Colour(0xff252525), line.getX(), centerY, juce::Colour(0xff252525), line.getRight(), centerY, false);
            lightOffGradient.addColour(0.5f, juce::Colour(0xff303030));
            // draw light dip
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff101010), line.getX(), line.getY(), juce::Colour(0xff505050), line.getX(), line.getBottom(), false));
            g.drawRoundedRectangle(line.expanded(1.0f), 1.5f, 1.0f);
            if (button.getToggleState())
            {
                g.setGradientFill(lightOnGradient);
                g.fillRoundedRectangle(line, 1.5f);
                glow.drawForRectangle(g, line.toNearestInt());
            }
            else
            {
                g.setGradientFill(lightOffGradient);
                g.fillRoundedRectangle(line, 1.5f);
                // draw light rim
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff101010), line.getX(), line.getBottom(), juce::Colour(0xff505050), line.getX(), line.getY(), false));
                g.drawRoundedRectangle(line, 1.5f, 1.0f);
            }
            // draw label
            g.setColour(juce::Colours::grey);
            g.setFont(xxii.withHeight(14.0f));
            auto textArea = juce::Rectangle<float>(xPosition, yPosition + height + 10.0f, width, 20.0f);
            g.drawText(button.getName(), textArea, juce::Justification::centredTop, false);
        }
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override {}

    private:
        juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    SmallButton(const juce::String& name="")
    {
        setLookAndFeel(&laf);
        setName(name);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }
    ~SmallButton(){}

    void setHitArea(juce::Rectangle<float> r)
    {
        buttonRectangle = r;
    }

    bool hitTest(int x, int y)
    {
        auto point = juce::Point<int>(x, y);
        return(buttonRectangle.contains(point.toFloat()));
    }

private:
    juce::Rectangle<float> buttonRectangle;
    SmallButtonLaF laf;
};

/* Button with centered light box */
class BigButton : public juce::TextButton
{
public:
    class BigButtonLaF : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool, bool) override
        {
            button.setClickingTogglesState(true);
            // get dimensions
            auto area = button.getLocalBounds().toFloat();
            float width = area.getWidth() - 4.0f;
            float height = area.getHeight() - 4.0f;
            float xPosition = area.getX() + 2.0f;
            float yPosition = area.getY() + 2.0f;
            float centerX = width * 0.5f + xPosition;
            auto buttonArea = juce::Rectangle<float>(xPosition, yPosition, width, height);
            // set hit box
            BigButton* bigButton = static_cast<BigButton*>(&button);
            bigButton->setHitArea(buttonArea);
			bigButton = nullptr;
            // draw outline
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(buttonArea.expanded(2.0f), 4.0f);
            // draw button face
            auto innerGradient = juce::ColourGradient(juce::Colour(0xff303030), centerX, buttonArea.getY(),
                juce::Colour(0xff303030), centerX, buttonArea.getBottom(), false);
            innerGradient.addColour(0.5f, juce::Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(buttonArea, 3.0f);
            // draw rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), centerX, buttonArea.getY(),
                juce::Colour(0xff101010), centerX, buttonArea.getBottom(), false));
            g.drawRoundedRectangle(buttonArea, 3.0f, 1.0f);
            // draw light
            auto glow = juce::DropShadow(juce::Colour(0x88ff0000), 14, juce::Point<int>(0, 0));
            auto light = buttonArea.reduced(15.0f);
            auto lightOnGradient = juce::ColourGradient(juce::Colours::orange, light.getX(), light.getY(),
                juce::Colours::orange, light.getRight(), light.getBottom(), false);
            lightOnGradient.addColour(0.5f, juce::Colours::yellow);
            auto lightOffGradient = juce::ColourGradient(juce::Colour(0xff252525), light.getX(), light.getY(),
                juce::Colour(0xff252525), light.getRight(), light.getBottom(), false);
            lightOffGradient.addColour(0.5f, juce::Colour(0xff303030));
            // draw light dip
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff101010), light.getX(), light.getY(), juce::Colour(0xff505050), light.getX(), light.getBottom(), false));
            g.drawRoundedRectangle(light.expanded(1.0f), 1.5f, 1.0f);
            if (button.getToggleState())
            {
                g.setGradientFill(lightOnGradient);
            }
            else
            {
                g.setGradientFill(lightOffGradient);
            }
            g.fillRoundedRectangle(light, 2.0f);
            // draw light rim
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff505050), light.getCentreX(), light.getY(),
                juce::Colour(0xff101010), light.getCentreX(), light.getBottom(), false));
            g.drawRoundedRectangle(light, 2.0f, 1.0f);
            // draw glow
            if (button.getToggleState())
            {
                glow.drawForRectangle(g, light.toNearestInt());
            }
        }
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override {}
    };

    BigButton(const juce::String& name="")
    {
        setName(name);
        setLookAndFeel(&laf);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }
    ~BigButton() {}

    void setHitArea(juce::Rectangle<float> r)
    {
        buttonRectangle = r;
    }

    bool hitTest(int x, int y)
    {
        auto point = juce::Point<int>(x, y);
        return(buttonRectangle.contains(point.toFloat()));
    }

private:
    juce::Rectangle<float> buttonRectangle;
    BigButtonLaF laf;
};

/* Simple label with grey font */
class GreyLabel : public juce::Label
{
public:
    class LabelLaF : public juce::LookAndFeel_V4
    {
    public:
        void drawLabel(juce::Graphics& g, juce::Label& label) override
        {
            // set font
            g.setFont(xxii.withHeight(14.0f));
            g.setColour(juce::Colours::grey);
            // draw text
            auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            g.drawText(label.getText(), textArea, juce::Justification::centred, false);
        }
    private:
        juce::Font xxii = { juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
    };

    GreyLabel(const juce::String& name="")
    {
        setText(name, juce::NotificationType::dontSendNotification);
        setLookAndFeel(&laf);
    }
    ~GreyLabel() {}

private:
    LabelLaF laf;
};

/* Label with enclosing horizontal bracket */
class MultiLabel : public juce::Component
{
public:
    MultiLabel(const juce::String& t)
    {
        text = t;
    }
    ~MultiLabel() {}
    void paint(juce::Graphics& g)
    {
        auto area = getLocalBounds().toFloat();
        area.reduce(2.0f, 0.0f);
        // draw text
        g.setFont(xxii.withHeight(14.0f));
        g.setColour(juce::Colours::grey);
        float textWidth = g.getCurrentFont().getStringWidthFloat(text) + 10.0f;
        auto textArea = juce::Rectangle<float>(area.getCentreX() - (textWidth * 0.5f), area.getY(), textWidth, 16.0f);
        g.drawText(text, textArea, juce::Justification::centred, false);
        // draw lines
        juce::Path lines;
        lines.startNewSubPath(area.getBottomLeft());
        lines.lineTo(area.getX(), textArea.getCentreY());
        lines.lineTo(textArea.getX(), textArea.getCentreY());
        lines.startNewSubPath(textArea.getRight(), textArea.getCentreY());
        lines.lineTo(area.getRight(), textArea.getCentreY());
        lines.lineTo(area.getBottomRight());
        g.strokePath(lines, { 1, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
    }

private:
    juce::String text;
    juce::Font xxii{ juce::Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, BinaryData::XXIIAvenRegular_ttfSize) };
};

/* Dual r/unixporn style powerline objects */
class PowerLine : public juce::Component
{
public:
    PowerLine(juce::String textA, juce::String textB, float inputHeight)
    {
        text[0] = textA;
        text[1] = textB;
        height = inputHeight;
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    void paint(juce::Graphics& g)
    {
        g.setFont(domitian.withHeight(height * 0.6f));
        float offset = height * 0.5f;
        // get widths and x positions
        for (int i = 0; i < 2; i++)
        {
            width[i] = g.getCurrentFont().getStringWidthFloat(text[i]) + 35.0f;
        }
        xPosition[0] = 10.0f;
        xPosition[1] = xPosition[0] + width[0] + int(height / 4);
        // draw shapes - 2nd first so shadow is underneath 1st
        for (int i = 1; i >= 0; i--)
        {
            // create powerline path
            juce::Path shape;
            shape.startNewSubPath(xPosition[i], 0);
            shape.lineTo(xPosition[i] + width[i], 0);
            shape.lineTo(xPosition[i] + width[i] + offset, offset);
            shape.lineTo(xPosition[i] + width[i], height);
            shape.lineTo(xPosition[i], height);
            shape.lineTo(xPosition[i] + offset, offset);
            shape.closeSubPath();
            // create dropshadow
            juce::DropShadow dropShadow = juce::DropShadow(juce::Colours::black, 10, juce::Point<int>(-2, 2));
            dropShadow.drawForPath(g, shape);
            // create powerline shape
            g.setGradientFill(juce::ColourGradient(shapeColors[i], xPosition[i], 0, shapeColors[i + 2], xPosition[i], height, false));
            g.fillPath(shape);
            // create powerline edge
            auto edgeGradient = juce::ColourGradient(juce::Colour(0xffe0e0e0), xPosition[i], 0, juce::Colour(0xff707070), xPosition[i], height, false);
            edgeGradient.addColour(0.49f, juce::Colour(0xffe0e0e0));
            edgeGradient.addColour(0.51f, juce::Colour(0xff707070));
            g.setGradientFill(edgeGradient);
            g.strokePath(shape, { 1.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::square });
            // draw text
            g.setColour(juce::Colour(0xff1c1f24));
            g.drawText(text[i], int(xPosition[i] + offset), 0, int(width[i] - offset), (int)height, juce::Justification::centred, false);
        }
    }

private:
    juce::String text[2];
    float xPosition[2];
    float width[2];
    float height;
    juce::Font domitian{ juce::Typeface::createSystemTypefaceFor(BinaryData::DomitianRoman_otf, BinaryData::DomitianRoman_otfSize) };
    juce::Colour shapeColors[4] = { juce::Colour(0xff51afef),   // blue
                                    juce::Colour(0xff818e96),   // grey
                                    juce::Colour(0xff306990),   // dark blue
                                    juce::Colour(0xff4b5156) }; // dark grey
};

/* Wrapper for using an image as the background */
class BgImage : public juce::Component
{
public:
    BgImage()
    {
        background = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }
    void paint(juce::Graphics& g)
    {
        g.drawImageAt(background, 0, 0);
    }
private:
    juce::Image background;
};
