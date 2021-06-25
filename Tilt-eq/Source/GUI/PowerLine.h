/*
*   Powerline Module
*       by Jacob Curtis
*       Created/tested on Windows using Reaper VST3
*
*  Draws a powerline shape with text
*
*  Implementation Instructions:
*
*   In PluginEditor.h
*		#include "PowerLine.h"
*       create object PowerLine powerLine
*	In PluginEditor.cpp, in paint method:
*		call powerLine.drawPowerline() with arguments
*
*/

#pragma once
#include <JuceHeader.h>

class PowerLine
{
public:
    void drawPowerLine(juce::Graphics& g, float x, float y, float width, float height, int shapeColor, int textColor, juce::String text)
    {
        // calculate offset
        float offset = height / 2.0f;
        // convert color arguments from int to color
            //  0     1     2      3       4      5      6      7      8
            //black, red, green, yellow, blue, magenta, cyan, white, grey
        juce::Colour themeColors[9] = { juce::Colour(0xff1c1f24), juce::Colour(0xffff6c6b), juce::Colour(0xff98be65),
                                        juce::Colour(0xffda8548), juce::Colour(0xff51afef), juce::Colour(0xffc678dd),
                                        juce::Colour(0xff5699af), juce::Colour(0xffd0def4),juce::Colour(0xff818e96) };
        // create powerline path
        juce::Path p;
        p.startNewSubPath(x, y);
        p.lineTo(x + width, y);
        p.lineTo(x + width + offset, y + offset);
        p.lineTo(x + width, y + height);
        p.lineTo(x, y + height);
        p.lineTo(x + offset, y + offset);
        p.closeSubPath();
        // create dropshadow
        juce::DropShadow dropShadow = juce::DropShadow(juce::Colours::black, 10, juce::Point<int>(-2, 2));
        dropShadow.drawForPath(g, p);
        // create powerline shape
        g.setColour(themeColors[shapeColor]);
        g.fillPath(p);
        // set font
        g.setFont(juce::Font("Constantia", height - 12.0f, 0));
        // draw text shadow
        g.setColour(themeColors[shapeColor].brighter());
        auto textArea = juce::Rectangle<float>(x + offset, y + 1, width - offset, height).toNearestInt();
        g.drawText(text, textArea, juce::Justification::centred, false);
        // draw text
        g.setColour(themeColors[textColor]);
        g.drawText(text, textArea, juce::Justification::centred, false);
    }
};