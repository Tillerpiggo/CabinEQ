/*
  ==============================================================================

    Stroke.cpp
    Created: 13 Nov 2024 10:35:58pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Stroke.h"

Stroke::Stroke (std::vector<NoisePoint> points)
    : points (points)
{
}

NoisePoint Stroke::positionAtTime (float time) const
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
    
    auto belowPt = points[belowIdx];
    auto abovePt = points[aboveIdx];
    
    float posX = belowPt.x * belowPercent + abovePt.x * abovePercent;
    float posY = belowPt.y * belowPercent + abovePt.y * abovePercent;
    float posV = belowPt.vol * belowPercent + abovePt.vol * abovePercent;
    
    // Linearly interpolate the point
    return { posX, posY, posV };
}

const std::vector<NoisePoint>& Stroke::getPoints() const
{
    return points;
}

std::pair<NoisePoint, NoisePoint> Stroke::getEndPoints() const
{
    return { points[0], points[points.size() - 1] };
}
