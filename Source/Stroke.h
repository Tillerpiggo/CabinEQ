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
    Stroke (std::vector<juce::Point<float>> points);
    juce::Point<float> positionAtTime (float time) const; // time from [0, 1]
    const std::vector<juce::Point<float>>& getPoints() const;
    std::pair<juce::Point<float>, juce::Point<float>> getEndPoints() const; // returns the start and end points
    
private:
    std::vector<juce::Point<float>> points; // for now, assume it moves between all points evenly and continuously over time
};
