/*
  ==============================================================================

    Stroke.h
    Created: 13 Nov 2024 10:35:58pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a single 3D point, with x in [-1, 1], y in [-1, 1], and volume (z) in [0, 1]
class NoisePoint
{
public:
    NoisePoint (float x, float y, float vol = 1)
        : x (x), y (y), vol (vol)
    {}
    
    juce::Point<float> point()
    {
        return { x, y };
    }
    
    float x;
    float y;
    float vol;
};

// This class represents a single stroke in 2D space over some time. It can say where the "brush" is at each point in time from [0, 1] in 2D space with x in [-1, 1] and y in [-1, 1]
class Stroke
{
public:
    Stroke (std::vector<NoisePoint> points);
    NoisePoint positionAtTime (float time) const; // time from [0, 1]
    const std::vector<NoisePoint>& getPoints() const;
    std::pair<NoisePoint, NoisePoint> getEndPoints() const; // returns the start and end points
    float getLength() const;
    
private:
    void updateLength(); // computes length from the given points
    
    std::vector<NoisePoint> points; // for now, assume it moves between all points evenly and continuously over time
    float length; // total length between all points
};
