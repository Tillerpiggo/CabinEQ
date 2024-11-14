/*
  ==============================================================================

    Glyph.h
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Stroke.h"

// This represents a Glyph, which is simply a sequence of strokes. It can tell you the needed position at a given time
class Glyph
{
public:
    Glyph (std::vector<Stroke> initialStrokes = {});
    std::pair<float, float> positionAtTime (float time); // from [0, 1)
    const std::vector<Stroke>& getStrokes();
    
private:
    std::vector<Stroke> strokes;
};
