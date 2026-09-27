/*
  ==============================================================================

    Theme.h

    CabinEQ's colours, fonts and icons.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Theme
{
    // Surfaces, darkest first
    inline const juce::Colour background   { 0xff0e0f12 };
    inline const juce::Colour graph        { 0xff0b0c0f };
    inline const juce::Colour sidebar      { 0xff14161a };
    inline const juce::Colour panel        { 0xff1a1d22 };
    inline const juce::Colour raised       { 0xff23272e };
    inline const juce::Colour raisedHover  { 0xff2c313a };
    inline const juce::Colour border       { 0xff2a2e36 };

    // Text
    inline const juce::Colour text         { 0xffe8eaed };
    inline const juce::Colour textDim      { 0xff9aa0a8 };
    inline const juce::Colour textFaint    { 0xff5f6570 };

    // Accents
    inline const juce::Colour accent       { 0xff9d8cff };
    inline const juce::Colour accentBright { 0xffc3b8ff };
    inline const juce::Colour danger       { 0xffff6b6b };
    inline const juce::Colour leftChannel  { 0xff5ab0ff };
    inline const juce::Colour rightChannel { 0xffff8a6b };

    // Graph
    inline const juce::Colour gridLine     { 0xff1c1f25 };
    inline const juce::Colour gridLineMajor{ 0xff262a31 };
    inline const juce::Colour zeroLine     { 0xff3a3f48 };
    inline const juce::Colour spectrum     { 0xff2b3a4f };

    /// Each band keeps its colour, picked by its id
    inline juce::Colour bandColour (int bandId)
    {
        static const juce::Colour colours[] {
            juce::Colour (0xffff7a6b), juce::Colour (0xffffb454), juce::Colour (0xfff2dc6b), juce::Colour (0xff7ee081),
            juce::Colour (0xff4fd1c5), juce::Colour (0xff5ab0ff), juce::Colour (0xff9d8cff), juce::Colour (0xfff28bd6)
        };
        return colours[(size_t) (std::abs (bandId) % 8)];
    }

    inline juce::Typeface::Ptr typeface()
    {
        static auto face = juce::Typeface::createSystemTypefaceFor (BinaryData::Inter_ttf, BinaryData::Inter_ttfSize);
        return face;
    }

    inline juce::Font font (float height, bool bold = false)
    {
        juce::Font result { juce::FontOptions (typeface()).withHeight (height) };
        return bold ? result.boldened() : result;
    }

    inline float textWidth (const juce::Font& font, const juce::String& text)
    {
        return juce::GlyphArrangement::getStringWidth (font, text);
    }

    constexpr float cornerRadius = 6.0f;
    constexpr int topBarHeight = 52;
    constexpr int sidebarWidth = 232;
    constexpr int narrowSidebarWidth = 196; // for windows under 1000 pixels wide
    inline int sidebarWidthFor (int windowWidth) { return windowWidth < 1000 ? narrowSidebarWidth : sidebarWidth; }
    constexpr int inspectorHeight = 64;
}

/// Simple line icons, drawn in a 24 x 24 box.
namespace Icons
{
    inline juce::Path power()
    {
        juce::Path p;
        p.addCentredArc (12, 13, 7.5f, 7.5f, 0, juce::degreesToRadians (40.0f), juce::degreesToRadians (320.0f), true);
        p.startNewSubPath (12, 3.5f);
        p.lineTo (12, 11.5f);
        return p;
    }

    inline juce::Path undo()
    {
        juce::Path p;
        p.startNewSubPath (9, 5);
        p.lineTo (4.5f, 9.5f);
        p.lineTo (9, 14);
        p.startNewSubPath (4.5f, 9.5f);
        p.lineTo (14, 9.5f);
        p.cubicTo (17.5f, 9.5f, 20, 12, 20, 15);
        p.cubicTo (20, 18, 17.5f, 20, 14, 20);
        p.lineTo (10, 20);
        return p;
    }

    inline juce::Path redo()
    {
        auto p = undo();
        p.applyTransform (juce::AffineTransform::scale (-1.0f, 1.0f, 12.0f, 12.0f));
        return p;
    }

    inline juce::Path plus()
    {
        juce::Path p;
        p.startNewSubPath (12, 5);
        p.lineTo (12, 19);
        p.startNewSubPath (5, 12);
        p.lineTo (19, 12);
        return p;
    }

    inline juce::Path settings()
    {
        // Three sliders
        juce::Path p;
        for (auto [y, knob] : { std::pair { 6.0f, 15.0f }, std::pair { 12.0f, 8.0f }, std::pair { 18.0f, 13.0f } })
        {
            p.startNewSubPath (4, y);
            p.lineTo (20, y);
            p.addEllipse (knob - 2.0f, y - 2.0f, 4.0f, 4.0f);
        }
        return p;
    }

    inline juce::Path crossfeed()
    {
        // Two overlapping circles, one per ear
        juce::Path p;
        p.addEllipse (3, 7, 10, 10);
        p.addEllipse (11, 7, 10, 10);
        return p;
    }

    inline juce::Path ellipsis()
    {
        juce::Path p;
        for (float x : { 6.0f, 12.0f, 18.0f })
            p.addEllipse (x - 1.5f, 10.5f, 3.0f, 3.0f);
        return p;
    }

    /// Scales an icon to fit the area and strokes it.
    inline void draw (juce::Graphics& g, const juce::Path& icon, juce::Rectangle<float> area, juce::Colour colour, float thickness = 1.6f)
    {
        const float size = std::min (area.getWidth(), area.getHeight());
        auto transform = juce::AffineTransform::scale (size / 24.0f).translated (area.getCentreX() - size / 2.0f, area.getCentreY() - size / 2.0f);
        g.setColour (colour);
        // The stroke is scaled along with the path, so undo that to get the thickness on screen
        g.strokePath (icon, juce::PathStrokeType (thickness * 24.0f / size, juce::PathStrokeType::curved, juce::PathStrokeType::rounded), transform);
    }
}
