/*
  ==============================================================================

    Stroke.h
    Created: 13 Nov 2024 10:35:58pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a single stroke in 2D space over some time. It can say where the "brush" is at each point in time from [0, 1] in 2D space with x in [-1, 1] and y in [-1, 1]
class Stroke
{
public:
    Stroke (std::vector<std::pair<float, float>> points);
    std::pair<float, float> positionAtTime (float time); // time from [0, 1]
    
private:
    std::vector<std::pair<float, float>> points; // for now, assume it moves between all points evenly and continuously over time
};
