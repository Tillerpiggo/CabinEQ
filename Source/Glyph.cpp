/*
  ==============================================================================

    Glyph.cpp
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Glyph.h"

Glyph::Glyph (std::vector<Stroke> initialStrokes)
    : strokes (initialStrokes)
{
}

std::pair<juce::Point<float>, float> Glyph::positionAtTime (float time)
{
    if (time < 0 || time >= 1)
    {
        std::cerr << "Called positionAtTime in Glyph with invalid time outside of [0, 1). (time=" << time << ")" << std::endl;
        return {{ 0.0f, 0.0f }, 0.0f };
    }
    
    // Figure out which stroke we're on
    float strokeTime = time * static_cast<float> (strokes.size());
    int strokeIdx = floor (strokeTime);
    float strokeProgress = strokeTime - static_cast<float> (strokeIdx);
    return { strokes[strokeIdx].positionAtTime (strokeProgress), strokeProgress };
}

const std::vector<Stroke>& Glyph::getStrokes()
{
    return strokes;
}
