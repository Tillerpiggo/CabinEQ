/*
  ==============================================================================

    Stroke.cpp
    Created: 13 Nov 2024 10:35:58pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Stroke.h"

Stroke::Stroke (std::vector<std::pair<float, float>> points)
    : points (points)
{
}

juce::Point<float> Stroke::positionAtTime (float time)
{
    if (time < 0 || time > 1)
    {
        std::cerr << "positionAtTime called in Stroke with invalid time outside of range [0, 1] (time=" << time << ")" << std::endl;
        return { 0.0f, 0.0f };
    }
    
    // Get the surrounding points
    float timeIdx = time * (points.size() - 1.0f);
    float belowIdx = floor (timeIdx);
    float aboveIdx = ceil (timeIdx);
    float belowPercent = aboveIdx - timeIdx;
    float abovePercent = 1.0f - belowPercent;
    
    std::pair<float, float> belowPt = points[belowIdx];
    std::pair<float, float> abovePt = points[aboveIdx];
    
    float posX = belowPt.first * belowPercent + abovePt.first * abovePercent;
    float posY = belowPt.second * belowPercent + abovePt.second * abovePercent;
    
    // Linearly interpolate the point
    return { posX, posY };
}

std::pair<juce::Point<float>, juce::Point<float>> Stroke::getEndPoints() const
{
    juce::Point<float> startPoint { points[0].first, points[0].second };
    juce::Point<float> endPoint { points[points.size() - 1].first, points[points.size() - 1].second };
    
    return { startPoint, endPoint };
}
