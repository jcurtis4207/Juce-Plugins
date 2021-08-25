/*
*   GUI Components Modules
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*/

#pragma once
#include <JuceHeader.h>

using namespace juce;

/* SSL-style knob with label underneath */
class SmallKnob : public Slider
{
public:
    class SmallKnobLaF : public LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(Slider&) override
        {
            return BubbleComponent::above;
        }

        Font getSliderPopupFont(Slider&) override
        {
            return Font(12.0f, Font::plain);
        }

        void drawBubble(Graphics& g, BubbleComponent&, const Point<float>&, 
            const Rectangle<float>& body) override
        {
            g.setColour(Colours::black);
            g.fillRect(body);
        }

        void drawShadow(Graphics& g, Rectangle<float> circleArea)
        {
            Path shadow;
            shadow.addEllipse(circleArea.reduced(4.0f));
            dropShadow.drawForPath(g, shadow);
        }

        void drawBumps(Graphics& g, Rectangle<float> circleArea, float angle)
        {
            #define numBumps 6
            const float bumpGap = 0.15f;
            const auto bumpFill = Rectangle<float>(
                circleArea.getCentre().getPointOnCircumference(circleArea.getWidth() * 0.5f, 
                    (float_Pi / 3.0f) + bumpGap),
                circleArea.getCentre().getPointOnCircumference(circleArea.getWidth() * 0.5f, 
                    (2 * float_Pi / 3.0f) - bumpGap).translated(-10.0f, 0.0f));
            Path bumps;
            for (int i = 0; i < numBumps; i++)
            {
                bumps.addPieSegment(circleArea, bumpGap, (float_Pi / 3.0f) - bumpGap, 0.0f);
                bumps.addRectangle(bumpFill);
                bumps.applyTransform(AffineTransform::rotation(float_Pi / 3.0f, 
                    circleArea.getCentreX(), circleArea.getCentreY()));
            }
            bumps.applyTransform(AffineTransform::rotation(angle, 
                circleArea.getCentreX(), circleArea.getCentreY()));
            g.setGradientFill(ColourGradient(Colour(0xffb0b0b0), 0, 0, 
                Colour(0xff303030), 0, circleArea.getHeight(), false));
            g.fillPath(bumps);
        }

        void drawKnobFace(Graphics& g, Rectangle<float> circleArea)
        {
            g.drawEllipse(circleArea.reduced(4.0f), 4.0f);
            auto innerGradient = ColourGradient(Colour(0xff303030), 0, 0,
                Colour(0xff303030), 0, circleArea.getHeight(), false);
            innerGradient.addColour(0.5f, Colour(0xff202020));
            g.setGradientFill(innerGradient);
            g.fillEllipse(circleArea.reduced(4.0f));
            g.setGradientFill(ColourGradient(Colour(0xff505050), 0, 0,
                Colour(0xff101010), 0, circleArea.getHeight(), false));
            g.drawEllipse(circleArea.reduced(4.0f), 2.0f);
        }

        void drawPointer(Graphics& g, juce::Rectangle<float> circleArea, float angle)
        {
            Path pointer;
            const float pointerLength = circleArea.getCentreY() * 0.8f;
            const float pointerThickness = 4.0f;
            pointer.addRectangle(circleArea.getCentreX() - (pointerThickness * 0.5f), 
                4.0f, pointerThickness, pointerLength);
            pointer.applyTransform(AffineTransform::rotation(angle, 
                circleArea.getCentreX(), circleArea.getCentreY()));
            g.setColour(Colour(0xffa0a0a0));
            g.fillPath(pointer);
        }

        void drawLabel(Graphics& g, Rectangle<float> circleArea, float textHeight, String labelText)
        {
            g.setColour(Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            g.drawText(labelText, Rectangle<float>(0, circleArea.getBottom() + 10.0f, 
                circleArea.getWidth(), textHeight), Justification::centredTop, false);
        }

        void drawRotarySlider(Graphics& g, int, int, int width, int height, float sliderPos, 
            float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
        {
            slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            const float textHeight = 14.0f;
            const float diameter = jmin(static_cast<float>(height - textHeight), 
                static_cast<float>(width));
            const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            const auto circleArea = Rectangle<float>((width * 0.5f) - (diameter * 0.5f), 
                0.0f, diameter, diameter);
            drawShadow(g, circleArea);
            drawBumps(g, circleArea, angle);
            drawKnobFace(g, circleArea);
            drawPointer(g, circleArea, angle);
            drawLabel(g, circleArea, textHeight, slider.getName());
            // set hit box
            SmallKnob* knob = static_cast<SmallKnob*>(&slider);
            knob->setHitArea(circleArea.getCentre(), circleArea.getWidth() * 0.5f);
        }
    private:
        const DropShadow dropShadow{ Colour(0xff000000), 15, Point<int>(0, 10) };
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    SmallKnob(const String& name="", const String& suffix="")
    {
        setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        setLookAndFeel(&laf);
        setName(name);
        setTextValueSuffix(String(" ")+suffix);
        setPaintingIsUnclipped(true);
    }

    void setHitArea(Point<float> p, float r)
    {
        setBufferedToImage(true);
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        const auto input = Point<int>(x, y).toFloat();
        const auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }
    
private:
    Point<float> center{ 0.0f, 0.0f };
    float knobRadius{ 0.0f };
    SmallKnobLaF laf;
};

/* Neve-style outer knob for nested knob setup */
class OuterKnob : public Slider
{
public:
    class OuterKnobLaF : public LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(Slider&) override
        {
            return BubbleComponent::above;
        }

        Font getSliderPopupFont(Slider&) override
        {
            return Font(12.0f, Font::plain);
        }

        void drawBubble(Graphics& g, BubbleComponent&, const Point<float>&, 
            const Rectangle<float>& body) override
        {
            g.setColour(Colours::black);
            g.fillRect(body);
        }

        void drawKnobFace(Graphics& g, Rectangle<float> circleArea)
        {
            // create shadow
            Path shadow;
            shadow.addEllipse(circleArea.reduced(2.0f));
            dropShadow.drawForPath(g, shadow);
            // draw knob face
            g.setGradientFill(ColourGradient(Colour(0xffb0b0b0), 0, 0,
                Colour(0xff303030), 0, circleArea.getWidth(), false));
            g.fillEllipse(circleArea);
            g.setColour(Colours::black);
            g.drawEllipse(circleArea, 1.0f);
        }

        void drawKnobHole(Graphics& g, Rectangle<float> circleArea)
        {
            // draw hole
            g.setColour(Colours::black);
            auto hole = Rectangle<float>(circleArea.getCentreX() - 28, 
                circleArea.getCentreY() - 28, 56, 56);
            g.fillEllipse(hole);
            // draw hole rim
            hole.expand(1.0f, 1.0f);
            g.setGradientFill(ColourGradient(Colour(0xff303030), hole.getX(), hole.getY(),
                Colour(0xffb0b0b0), hole.getX(), hole.getBottom(), false));
            g.drawEllipse(hole, 2.0f);
        }

        void drawPointer(Graphics& g, float angle, float center)
        {
            Path pointer;
            pointer.addEllipse(center - 2.5f, 3.0f, 5.0f, 5.0f);
            pointer.applyTransform(AffineTransform::rotation(angle, center, center));
            g.setColour(Colour(0xff202020));
            g.fillPath(pointer);
        }

        void drawRotarySlider(Graphics& g, int, int, int width, int height, 
            float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
        {
            slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            const float textHeight = 14.0f;
            const float diameter = jmin(static_cast<float>(height - textHeight), 
                static_cast<float>(width));
            const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            const auto circleArea = Rectangle<float>((width * 0.5f) - (diameter * 0.5f), 
                0.0f, diameter, diameter);
            drawKnobFace(g, circleArea);
            drawKnobHole(g, circleArea);
            drawPointer(g, angle, circleArea.getCentreX());
            // set hit box
            OuterKnob* knob = static_cast<OuterKnob*>(&slider);
            knob->setHitArea(Point<float>(circleArea.getCentreX(), 
                circleArea.getCentreY()), diameter * 0.5f);
        }

    private:
        const DropShadow dropShadow{ Colour(0xff000000), 10, Point<int>(0, 5) };
    };

    OuterKnob(const String& suffix="")
    {
        setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        setTextValueSuffix(String(" ") + suffix);
        setLookAndFeel(&laf);
    }

    void setHitArea(Point<float> p, float r)
    {
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        const auto input = Point<int>(x, y).toFloat();
        const auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }

    // get bounding rectangle for inner knob
    Rectangle<int> getInnerArea()
    {
        const auto area = getBounds();
        const int xPosition = area.getX() + (area.getWidth() / 2) - 25;
        const int yPosition = area.getY() + (area.getWidth() / 2) - 25;
        return Rectangle<int>(xPosition, yPosition, 50, 80);
    }

private:
    Point<float> center{ 0.0f, 0.0f };
    float knobRadius{ 0.0f };
    OuterKnobLaF laf;
};

/* Knob with illuminated ticks and label underneath */
class BigKnob : public Slider
{
public:
    class BigKnobLaF : public LookAndFeel_V4
    {
    public:
        int getSliderPopupPlacement(Slider&) override
        {
            return BubbleComponent::above;
        }

        Font getSliderPopupFont(Slider&) override
        {
            return Font(12.0f, Font::plain);
        }

        void drawBubble(Graphics& g, BubbleComponent&, const Point<float>&, 
            const Rectangle<float>& body) override
        {
            g.setColour(Colours::black);
            g.fillRect(body);
        }

        void drawMarks(Graphics& g, float rotaryStartAngle, float rotaryEndAngle, 
            float angle, float centerX, float centerY)
        {
            #define numMarks 12
            const float markAngle = (rotaryEndAngle - rotaryStartAngle) / numMarks;
            const int activePosition = static_cast<int>((angle - rotaryStartAngle) / 
                ((rotaryEndAngle - rotaryStartAngle) / static_cast<float>(numMarks)));
            Path activeMarks, inactiveMarks;
            for (int i = numMarks; i >= 0; i--)
            {
                if (i > activePosition)
                    inactiveMarks.addRoundedRectangle(centerX - 1.0f, 0.0f, 2.0f, 8.0f, 1.0f);
                else
                    activeMarks.addRoundedRectangle(centerX - 1.0f, 0.0f, 2.0f, 8.0f, 1.0f);
                activeMarks.applyTransform(AffineTransform::rotation(markAngle, centerX, centerY));
                inactiveMarks.applyTransform(AffineTransform::rotation(markAngle, centerX, centerY));
            }
            activeMarks.applyTransform(AffineTransform::rotation(rotaryStartAngle -
                ((rotaryEndAngle - rotaryStartAngle) / numMarks), centerX, centerY));
            inactiveMarks.applyTransform(AffineTransform::rotation(rotaryStartAngle -
                ((rotaryEndAngle - rotaryStartAngle) / numMarks), centerX, centerY));
            // fill marks
            g.setColour(Colour(0xff404040));
            g.fillPath(inactiveMarks);
            g.setColour(Colours::yellow);
            g.fillPath(activeMarks);
            glow.drawForPath(g, activeMarks);
        }

        void drawKnobFace(Graphics& g, Rectangle<float> circleArea)
        {
            // create shadow
            Path shadow;
            shadow.addEllipse(circleArea);
            dropShadow.drawForPath(g, shadow);
            // draw inner circle
            auto innerGradient = ColourGradient(Colour(0xff303030), 0, circleArea.getY(),
                Colour(0xff303030), 0, circleArea.getBottom(), false);
            innerGradient.addColour(0.5f, Colour(0xff202020));
            g.setGradientFill(innerGradient);
            g.fillEllipse(circleArea);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050), 0, circleArea.getY(),
                Colour(0xff000000), 0, circleArea.getBottom(), false));
            g.drawEllipse(circleArea.reduced(1.0f), 2.0f);
        }

        void drawPointer(Graphics& g, float angle, float centerX, float centerY)
        {
            Path pointer;
            pointer.addTriangle(centerX, 22.0f, centerX - 4.0f, 30.0f, centerX + 4.0f, 30.0f);
            pointer.applyTransform(AffineTransform::rotation(angle, centerX, centerY));
            const auto pointerArea = pointer.getBounds();
            g.setGradientFill(ColourGradient(Colours::yellow, 
                pointerArea.getCentreX(), pointerArea.getCentreY(),
                Colours::orange, pointerArea.getX(), pointerArea.getY(), true));
            g.fillPath(pointer);
            g.setGradientFill(ColourGradient(Colour(0xff101010), 
                pointerArea.getCentreX(), pointerArea.getY(),
                Colour(0xff505050), pointerArea.getCentreX(), pointerArea.getBottom(), false));
            g.strokePath(pointer, { 1.0f, PathStrokeType::mitered, PathStrokeType::butt });
            glow.drawForPath(g, pointer);
        }

        void drawLabel(Graphics& g, Rectangle<float> circleArea, 
            float textHeight, String labelText)
        {
            g.setColour(Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            g.drawText(labelText, Rectangle<float>(circleArea.getX(), circleArea.getBottom() + 15.0f, 
                circleArea.getWidth(), textHeight), Justification::centredTop, false);
        }

        void drawRotarySlider(Graphics& g, int, int, int width, int height, float sliderPos, 
            float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
        {
            slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            slider.setPopupDisplayEnabled(true, true, nullptr);
            const float textHeight = 14.0f;
            const float diameter = jmin(static_cast<float>(height - textHeight), 
                static_cast<float>(width));
            const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto circleArea = Rectangle<float>((width * 0.5f) - (diameter * 0.5f), 
                0.0f, diameter, diameter);
            drawMarks(g, rotaryStartAngle, rotaryEndAngle, angle, 
                circleArea.getCentreX(), circleArea.getCentreY());
            circleArea.reduce(15.0f, 15.0f);
            drawKnobFace(g, circleArea);
            drawPointer(g, angle, circleArea.getCentreX(), circleArea.getCentreY());
            drawLabel(g, circleArea, textHeight, slider.getName());
            // set hit box
            BigKnob* bigKnob = static_cast<BigKnob*>(&slider);
            bigKnob->setHitArea(circleArea.getCentre(), circleArea.getWidth() * 0.5f);
        }

    private:
        const DropShadow glow{ Colour(0x99ff0000), 20, Point<int>(0, 0) };
        const DropShadow dropShadow{ Colour(0xff000000), 15, Point<int>(0, 10) };
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    BigKnob(const String& name="", const String& suffix="")
    {
        setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        setTextValueSuffix(String(" ") + suffix);
        setLookAndFeel(&laf);
        setName(name);
    }

    void setHitArea(Point<float> p, float r)
    {
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
        center = p;
        knobRadius = r;
    }

    bool hitTest(int x, int y) override
    {
        const auto input = Point<int>(x, y).toFloat();
        const auto distance = input.getDistanceFrom(center);
        return (distance < knobRadius);
    }

private:
    Point<float> center{ 0.0f, 0.0f };
    float knobRadius{ 0.0f };
    BigKnobLaF laf;
};

/* Slider with value on the thumb */
class VerticalSlider : public Slider
{
public:
    class VerticalSliderLaF : public LookAndFeel_V4
    {
    public:
        void drawTrack(Graphics& g, Rectangle<float> trackRectangle)
        {           
            g.setColour(Colours::black);
            g.fillRoundedRectangle(trackRectangle, 2.0f);
            g.setColour(Colour(0xff303030));
            g.drawRoundedRectangle(trackRectangle, 2.0f, 1.0f);
        }

        void drawThumb(Graphics& g, Rectangle<int> thumbBounds)
        {
            dropShadow.drawForRectangle(g, thumbBounds.reduced(2));
            // draw thumb
            auto thumbGradient = ColourGradient(Colour(0xff303030),
                Point<int>(thumbBounds.getX(), thumbBounds.getY()).toFloat(),
                Colour(0xff303030), Point<int>(thumbBounds.getX(), 
                    thumbBounds.getBottom()).toFloat(), false);
            thumbGradient.addColour(0.5f, Colour(0xff272727));
            g.setGradientFill(thumbGradient);
            g.fillRoundedRectangle(thumbBounds.toFloat(), 2.0f);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050),
                Point<int>(thumbBounds.getX(), thumbBounds.getY()).toFloat(),
                Colour(0xff101010), Point<int>(thumbBounds.getX(), 
                    thumbBounds.getBottom()).toFloat(), false));
            g.drawRoundedRectangle(thumbBounds.toFloat(), 2.0f, 1.0f);
        }

        void drawLabel(Graphics& g, Rectangle<int> thumbBounds, double value)
        {
            g.setColour(Colours::grey);
            g.setFont(xxii.withHeight(16.0f));
            g.drawText(String(value, 1), thumbBounds.translated(0, -1).toFloat(),
                Justification::centred, false);
        }

        void drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, 
            float, float, const Slider::SliderStyle, Slider& slider) override
        {
            slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
            const float trackWidth = 6.0f;
            const auto trackRectangle = Rectangle<float>(
                static_cast<float>(x + width * 0.5f) - (trackWidth * 0.5f), 
                static_cast<float>(y), trackWidth, static_cast<float>(height + y - 5));
            drawTrack(g, trackRectangle);
            const auto maxPoint = Point<float>(trackRectangle.getCentreX(), sliderPos).toInt();
            const auto thumbBounds = Rectangle<int>(40, 20).withCentre(maxPoint);
            drawThumb(g, thumbBounds);
            drawLabel(g, thumbBounds, slider.getValue());
            // set hit box
            VerticalSlider* verticalSlider = static_cast<VerticalSlider*>(&slider);
            verticalSlider->setHitArea(maxPoint);
        }
    private:
        const DropShadow dropShadow{ Colour(0xdd000000), 10, Point<int>(0, 5) };
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    VerticalSlider(const String& suffix="")
    {
        setSliderStyle(Slider::SliderStyle::LinearVertical);
        setTextValueSuffix(String(" ") + suffix);
        setLookAndFeel(&laf);
    }

    void setHitArea(Point<int> p)
    {
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
        center = p;
    }

    bool hitTest(int x, int y) override
    {
        const auto area = Rectangle<int>(40, 20).withCentre(center);
        const auto point = Point<int>(x, y);
        return(area.contains(point));
    }

private:
    Point<int> center;
    VerticalSliderLaF laf;
};

/* Immovable knob with label above */
class LinkKnob : public Slider
{
public:
    class LinkKnobLaF : public LookAndFeel_V4
    {
    public:
        void drawKnobFace(Graphics& g, Rectangle<float> boxArea)
        {
            //draw outline
            g.setColour(Colours::black);
            g.fillRoundedRectangle(boxArea.expanded(2.0f), 3.0f);
            // draw knob
            auto innerGradient = ColourGradient(Colour(0xff303030), boxArea.getX(), boxArea.getY(),
                Colour(0xff303030), boxArea.getX(), boxArea.getBottom(), false);
            innerGradient.addColour(0.5f, Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(boxArea, 3.0f);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050), boxArea.getX(), boxArea.getY(),
                Colour(0xff101010), boxArea.getX(), boxArea.getBottom(), false));
            g.drawRoundedRectangle(boxArea, 3.0f, 1.0f);
        }

        void drawTriangles(Graphics& g, Rectangle<float> boxArea)
        {
            g.setColour(Colours::grey);
            Path triangles;
            const float centerX = boxArea.getWidth() * 0.5f + boxArea.getX();
            const float centerY = boxArea.getHeight() * 0.5f + boxArea.getY();
            triangles.addTriangle(centerX - 1, boxArea.getY() + 7, centerX - 1, 
                boxArea.getBottom() - 7, boxArea.getX() + 5, centerY);
            triangles.addTriangle(centerX + 1, boxArea.getY() + 7, centerX + 1, 
                boxArea.getBottom() - 7, boxArea.getRight() - 5, centerY);
            g.fillPath(triangles);
        }

        void drawLabel(Graphics& g, Rectangle<float> boxArea, float textHeight)
        {
            g.setColour(Colours::grey);
            g.setFont(xxii.withHeight(textHeight));
            g.drawText("Link", 
                Rectangle<float>(boxArea.getX(), boxArea.getY() - textHeight, boxArea.getWidth(), textHeight),
                Justification::centred, false);
        }

        void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float,
            const float, const float, Slider& slider) override
        {
            slider.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
            const float textHeight = 14.0f;
            const float diameter = jmin(static_cast<float>(width), static_cast<float>(height - textHeight)) - 4.0f;
            const auto boxArea = Rectangle<float>(static_cast<float>(width / 2 + x) - (diameter * 0.5f), 
                static_cast<float>(y + textHeight), diameter, diameter);
            drawKnobFace(g, boxArea);
            drawTriangles(g, boxArea);
            drawLabel(g, boxArea, textHeight);
            // set hit box
            LinkKnob* linkKnob = static_cast<LinkKnob*>(&slider);
            linkKnob->setHitArea(boxArea);
        }
    private:
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    LinkKnob()
    {
        setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
        setLookAndFeel(&laf);
        setRange(-40.0, 40.0, 0.1);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    void setHitArea(Rectangle<float> r)
    {
        area = r;
    }

    bool hitTest(int x, int y)
    {
        const auto point = Point<int>(x, y);
        return(area.contains(point.toFloat()));
    }

private:
    Rectangle<float> area;
    LinkKnobLaF laf;
};

/* Horizontal button with light bar */
class SmallButton : public TextButton
{
public:
    class SmallButtonLaF : public LookAndFeel_V4
    {
        void drawButtonFace(Graphics& g, Rectangle<float> buttonArea)
        {
            // draw outline
            g.setColour(Colours::black);
            g.fillRoundedRectangle(buttonArea.expanded(2.0f), 3.0f);
            // draw face
            auto innerGradient = ColourGradient(Colour(0xff303030), 0, buttonArea.getY(),
                Colour(0xff303030), 0, buttonArea.getBottom(), false);
            innerGradient.addColour(0.5f, Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(buttonArea, 2.0f);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050), 0, buttonArea.getY(),
                Colour(0xff101010), 0, buttonArea.getBottom(), false));
            g.drawRoundedRectangle(buttonArea, 2.0f, 1.0f);
        }

        void drawLight(Graphics& g, Rectangle<float> buttonArea, bool toggle)
        {
            const auto line = Rectangle<float>(buttonArea.getX() + 5.0f, 
                buttonArea.getCentreY() - 1.5f, buttonArea.getWidth() - 10.0f, 3.0f);
            auto lightOnGradient = ColourGradient(Colours::orange, line.getX(), 0,
                Colours::orange, line.getRight(), 0, false);
            lightOnGradient.addColour(0.5f, Colours::yellow);
            auto lightOffGradient = ColourGradient(Colour(0xff252525), line.getX(), 0,
                Colour(0xff252525), line.getRight(), 0, false);
            lightOffGradient.addColour(0.5f, Colour(0xff303030));
            // draw dip
            g.setGradientFill(ColourGradient(Colour(0xff101010), line.getX(), line.getY(),
                Colour(0xff505050), line.getX(), line.getBottom(), false));
            g.drawRoundedRectangle(line.expanded(1.0f), 1.5f, 1.0f);
            // draw interior
            if (toggle)
            {
                g.setGradientFill(lightOnGradient);
                g.fillRoundedRectangle(line, 1.5f);
                glow.drawForRectangle(g, line.toNearestInt());
            }
            else
            {
                g.setGradientFill(lightOffGradient);
                g.fillRoundedRectangle(line, 1.5f);
                // draw rim
                g.setGradientFill(ColourGradient(Colour(0xff101010), line.getX(), line.getBottom(),
                    Colour(0xff505050), line.getX(), line.getY(), false));
                g.drawRoundedRectangle(line, 1.5f, 1.0f);
            }
        }

        void drawLabel(Graphics& g, Rectangle<float> buttonArea, String labelText)
        {
            g.setColour(Colours::grey);
            g.setFont(xxii.withHeight(14.0f));
            g.drawText(labelText, Rectangle<float>(buttonArea.getX(), buttonArea.getBottom() + 10.0f, 
                buttonArea.getWidth(), 20.0f), Justification::centredTop, false);
        }

        void drawButtonBackground(Graphics& g, Button& button, const Colour&, bool, bool) override
        {
            button.setClickingTogglesState(true);
            const auto area = button.getLocalBounds().toFloat();
            const auto buttonArea = area.reduced(2.0f).withHeight(20.0f);
            drawButtonFace(g, buttonArea);
            drawLight(g, buttonArea, button.getToggleState());
            drawLabel(g, buttonArea, button.getName());
            // set hit box
            SmallButton* smallButton = static_cast<SmallButton*>(&button);
            smallButton->setHitArea(buttonArea);
        }

        void drawButtonText(Graphics&, TextButton&, bool, bool) override {}

    private:
        const DropShadow glow{ Colour(0x66ff0000), 14, Point<int>(0, -5) };
        const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    SmallButton(const String& name="")
    {
        setLookAndFeel(&laf);
        setName(name);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    void setHitArea(Rectangle<float> r)
    {
        buttonRectangle = r;
    }

    bool hitTest(int x, int y)
    {
        const auto point = Point<int>(x, y);
        return(buttonRectangle.contains(point.toFloat()));
    }

private:
    Rectangle<float> buttonRectangle;
    SmallButtonLaF laf;
};

/* Button with centered light box */
class BigButton : public TextButton
{
public:
    class BigButtonLaF : public LookAndFeel_V4
    {
    public:
        void drawButtonFace(Graphics& g, Rectangle<float> buttonArea)
        {
            // draw outline
            g.setColour(Colours::black);
            g.fillRoundedRectangle(buttonArea.expanded(2.0f), 4.0f);
            // draw button face
            auto innerGradient = ColourGradient(Colour(0xff303030), 0, buttonArea.getY(),
                Colour(0xff303030), 0, buttonArea.getBottom(), false);
            innerGradient.addColour(0.5f, Colour(0xff272727));
            g.setGradientFill(innerGradient);
            g.fillRoundedRectangle(buttonArea, 3.0f);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050), 0, buttonArea.getY(),
                Colour(0xff101010), 0, buttonArea.getBottom(), false));
            g.drawRoundedRectangle(buttonArea, 3.0f, 1.0f);
        }

        void drawLight(Graphics& g, Rectangle<float> buttonArea, bool toggle)
        {
            const auto light = buttonArea.reduced(15.0f);
            auto lightOnGradient = ColourGradient(Colours::orange, light.getX(), light.getY(),
                Colours::orange, light.getRight(), light.getBottom(), false);
            lightOnGradient.addColour(0.5f, Colours::yellow);
            auto lightOffGradient = ColourGradient(Colour(0xff252525), light.getX(), light.getY(),
                Colour(0xff252525), light.getRight(), light.getBottom(), false);
            lightOffGradient.addColour(0.5f, Colour(0xff303030));
            // draw dip
            g.setGradientFill(ColourGradient(Colour(0xff101010), 0, light.getY(),
                Colour(0xff505050), 0, light.getBottom(), false));
            g.drawRoundedRectangle(light.expanded(1.0f), 1.5f, 1.0f);
            // fill interior
            g.setGradientFill((toggle) ? lightOnGradient : lightOffGradient);
            g.fillRoundedRectangle(light, 2.0f);
            // draw rim
            g.setGradientFill(ColourGradient(Colour(0xff505050), 0, light.getY(),
                Colour(0xff101010), 0, light.getBottom(), false));
            g.drawRoundedRectangle(light, 2.0f, 1.0f);
            // draw glow
            if (toggle)
            {
                glow.drawForRectangle(g, light.toNearestInt());
            }
        }

        void drawButtonBackground(Graphics& g, Button& button, const Colour&, bool, bool) override
        {
            button.setClickingTogglesState(true);
            const auto area = button.getLocalBounds().toFloat();
            const auto buttonArea = area.reduced(2.0f);
            drawButtonFace(g, buttonArea);
            drawLight(g, buttonArea, button.getToggleState());
            // set hit box
            BigButton* bigButton = static_cast<BigButton*>(&button);
            bigButton->setHitArea(buttonArea);
        }

        void drawButtonText(Graphics&, TextButton&, bool, bool) override {}

    private:
        const DropShadow glow{ Colour(0x88ff0000), 14, Point<int>(0, 0) };
    };

    BigButton(const String& name="")
    {
        setName(name);
        setLookAndFeel(&laf);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    void setHitArea(Rectangle<float> r)
    {
        buttonRectangle = r;
    }

    bool hitTest(int x, int y)
    {
        const auto point = Point<int>(x, y);
        return(buttonRectangle.contains(point.toFloat()));
    }

private:
    Rectangle<float> buttonRectangle;
    BigButtonLaF laf;
};

/* Simple label with grey font */
class GreyLabel : public Label
{
public:
    class LabelLaF : public LookAndFeel_V4
    {
    public:
        void drawLabel(Graphics& g, Label& label) override
        {
            g.setFont(xxii.withHeight(14.0f));
            g.setColour(Colours::grey);
            const auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());
            g.drawText(label.getText(), textArea, Justification::centred, false);
        }
    private:
        const Font xxii = { Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
            BinaryData::XXIIAvenRegular_ttfSize) };
    };

    GreyLabel(const String& name="")
    {
        setText(name, NotificationType::dontSendNotification);
        setLookAndFeel(&laf);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

private:
    LabelLaF laf;
};

/* Label with enclosing horizontal bracket */
class MultiLabel : public Component
{
public:
    MultiLabel(const String& t)
    {
        text = t;
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    Rectangle<float> drawText(Graphics& g, Rectangle<float> area)
    {
        g.setFont(xxii.withHeight(14.0f));
        g.setColour(Colours::grey);
        const float textWidth = g.getCurrentFont().getStringWidthFloat(text) + 10.0f;
        const auto textArea = Rectangle<float>(area.getCentreX() - (textWidth * 0.5f), 
            area.getY(), textWidth, 16.0f);
        g.drawText(text, textArea, Justification::centred, false);
        return textArea;
    }

    void drawLines(Graphics& g, Rectangle<float> area, Rectangle<float> textArea)
    {
        Path lines;
        lines.startNewSubPath(area.getBottomLeft());
        lines.lineTo(area.getX(), textArea.getCentreY());
        lines.lineTo(textArea.getX(), textArea.getCentreY());
        lines.startNewSubPath(textArea.getRight(), textArea.getCentreY());
        lines.lineTo(area.getRight(), textArea.getCentreY());
        lines.lineTo(area.getBottomRight());
        g.strokePath(lines, { 1, PathStrokeType::mitered, PathStrokeType::square });
    }

    void paint(Graphics& g)
    {
        const auto area = getLocalBounds().toFloat().reduced(2.0f, 0.0f);
        const auto textArea = drawText(g, area);
        drawLines(g, area, textArea);
    }

private:
    String text;
    const Font xxii{ Typeface::createSystemTypefaceFor(BinaryData::XXIIAvenRegular_ttf, 
        BinaryData::XXIIAvenRegular_ttfSize) };
};

/* Dual r/unixporn style powerline objects */
class PowerLine : public Component
{
#define numShapes 2
public:
    PowerLine(const String& textA, const String& textB, float inputHeight)
    {
        text = { textA, textB };
        height = inputHeight;
        offset = height * 0.5f;
        edgeGradient = ColourGradient(Colour(0xffe0e0e0), 0, 0,
            Colour(0xff707070), 0, height, false );
        edgeGradient.addColour(0.49f, Colour(0xffe0e0e0));
        edgeGradient.addColour(0.51f, Colour(0xff707070));
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }

    void getWidths(Graphics& g)
    {
        for (int i = 0; i < numShapes; i++)
        {
            width[i] = g.getCurrentFont().getStringWidthFloat(text[i]) + 35.0f;
        }
        xPosition[0] = 10.0f;
        xPosition[1] = xPosition[0] + width[0] + static_cast<int>(height / 4);
    }

    Path getShape(int i)
    {
        Path shape;
        shape.startNewSubPath(xPosition[i], 0);
        shape.lineTo(xPosition[i] + width[i], 0);
        shape.lineTo(xPosition[i] + width[i] + offset, offset);
        shape.lineTo(xPosition[i] + width[i], height);
        shape.lineTo(xPosition[i], height);
        shape.lineTo(xPosition[i] + offset, offset);
        shape.closeSubPath();
        return shape;
    }

    void paint(Graphics& g)
    {
        g.setFont(domitian.withHeight(height * 0.6f));
        getWidths(g);
        // draw shapes - 2nd first so shadow is underneath 1st
        for (int i = numShapes - 1; i >= 0; i--)
        {
            Path shape = getShape(i);
            dropShadow.drawForPath(g, shape);
            // create powerline shape
            g.setGradientFill(ColourGradient(shapeColors[i], xPosition[i], 0, 
                shapeColors[i + 2], xPosition[i], height, false));
            g.fillPath(shape);
            // create powerline edge
            g.setGradientFill(edgeGradient);
            g.strokePath(shape, { 1.0f, PathStrokeType::mitered, PathStrokeType::square });
            // draw text
            g.setColour(Colour(0xff1c1f24));
            g.drawText(text[i], Rectangle<float>(xPosition[i] + offset, 0, width[i] - offset, height), 
                Justification::centred, false);
        }
    }

private:
    std::array<String, numShapes> text;
    std::array<float, numShapes> xPosition{ 0.0f, 0.0f };
    std::array<float, numShapes> width{ 0.0f, 0.0f };
    float height;
    float offset;
    const DropShadow dropShadow{ Colours::black, 10, Point<int>(-2, 2) };
    const Font domitian{ Typeface::createSystemTypefaceFor(BinaryData::DomitianRoman_otf, 
        BinaryData::DomitianRoman_otfSize) };
    ColourGradient edgeGradient;
    const std::array<Colour, 4> shapeColors { 
        Colour(0xff51afef),   // blue
        Colour(0xff818e96),   // grey
        Colour(0xff306990),   // dark blue
        Colour(0xff4b5156) }; // dark grey
};

/* Wrapper for using an image as the background */
class BgImage : public Component
{
public:
    BgImage()
    {
        background = ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
        setPaintingIsUnclipped(true);
        setBufferedToImage(true);
    }
    void paint(Graphics& g)
    {
        g.drawImageAt(background, 0, 0);
    }
private:
    Image background;
};
