/*
  ==============================================================================

    Glyph.cpp
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Glyph.h"

NoiseSource::NoiseSource (float x, float y, float startTime, float endTime, float rampLength)
    : x (x), y (y), startTime (startTime), endTime (endTime), rampLength (rampLength)
{
    rampLengthInTime = (endTime - startTime) * rampLength;
}

NoisePoint NoiseSource::noisePointAtTime (float time)
{
    return NoisePoint (x, y, volumeAtTime (time));
}

float NoiseSource::volumeAtTime (float time)
{
    if (time < startTime || time > endTime)
        return 0.0f;
    
    if (time > startTime + rampLengthInTime || time < endTime - rampLengthInTime)
        return 1.0f;
    
    float MIN_DB = -120.0f; // supposed to be about silent
    
    // Start ramp
    if (time < startTime + rampLengthInTime)
    {
        float rampPercent = (time - startTime) / rampLengthInTime;
        float volDB = MIN_DB * (1.0f - rampPercent);
        float gain = juce::Decibels::decibelsToGain (volDB);
        return gain;
    }
    
    // End ramp
    else
    {
        float rampPercent = (endTime - time) / rampLengthInTime;
        float volDB = MIN_DB * (1.0f - rampPercent);
        float gain = juce::Decibels::decibelsToGain (volDB);
        return gain;
    }
}

ArchetypalGlyph::ArchetypalGlyph (int id, std::vector<Stroke> initialStrokes)
    : id (id), strokes (initialStrokes)
{
}

NoisePoint ArchetypalGlyph::positionAtTime (float time) const
{
    if (time < 0 || time >= 1)
    {
        std::cerr << "Called positionAtTime in Glyph with invalid time outside of [0, 1). (time=" << time << ")" << std::endl;
        return { 0.0f, 0.0f };
    }
    
    // Figure out which stroke we're on
    float strokeTime = time * static_cast<float> (strokes.size());
    int strokeIdx = floor (strokeTime);
    float strokeProgress = strokeTime - static_cast<float> (strokeIdx);
    return strokes[strokeIdx].positionAtTime (strokeProgress);
}

const std::vector<Stroke>& ArchetypalGlyph::getStrokes() const
{
    return strokes;
}

const std::vector<NoisePoint> ArchetypalGlyph::getVertices() const
{
    // this will overlap the end points... whatever for now
    std::vector<NoisePoint> vertices;
    for (const auto& stroke : strokes)
    {
        for (const auto& point : stroke.getPoints())
        {
            vertices.push_back (point);
        }
    }
    return vertices;
}

const int ArchetypalGlyph::getId() const
{
    return id;
}

void ArchetypalGlyph::setCascadeSettings (bool isCascading, int density, float strokeOverlap, float dotOverlap)
{
    this->isCascading = isCascading;
    this->density = density;
    this->strokeOverlap = strokeOverlap;
    this->dotOverlap = dotOverlap;
    updateNoiseSources();
}

void ArchetypalGlyph::updateNoiseSources()
{
    
}

Glyph::Glyph (int id, ArchetypalGlyph archetype)
    : id (id), archetype (archetype)
{
}

Glyph::Glyph (int id, ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor)
    : id (id), archetype (archetype), horizontalSizeFactor (sizeFactor), verticalSizeFactor (sizeFactor), centerPos (centerPos)
{
    float minX = 1.0f;
    float maxX = -1.0f;
    float minY = 1.0f;
    float maxY = -1.0f;
    for (const auto& stroke : archetype.getStrokes())
    {
        for (const auto& point : stroke.getPoints())
        {
            minX = std::min (minX, point.x);
            maxX = std::max (maxX, point.x);
            minY = std::min (minY, point.y);
            maxY = std::max (maxY, point.y);
        }
    }
    
    width = std::max (0.01f, maxX - minX) / 2.0f;
    height = std::max (0.01f, maxY - minY) / 2.0f;
    
    moveGlyphWithinBounds();
}

NoisePoint Glyph::positionAtTime (float time) const
{
    return archetype.positionAtTime (time);
}

const std::vector<Stroke>& Glyph::getStrokes() const
{
    return archetype.getStrokes();
}

const std::vector<NoisePoint> Glyph::getVertices() const
{
    return archetype.getVertices();
}

ArchetypalGlyph Glyph::getArchetype() const
{
    return archetype;
}

std::pair<float, float> Glyph::getSizeFactor() const
{
    return { horizontalSizeFactor, verticalSizeFactor };
}

float Glyph::getVolume() const
{
    return volume;
}

juce::Point<float> Glyph::getCenterPos() const
{
    return centerPos;
}

int Glyph::getId() const
{
    return id;
}

void Glyph::setSizeFactor (float horizontalSizeFactor, float verticalSizeFactor)
{
    if (horizontalSizeFactor <= 0 || verticalSizeFactor <= 0)
    {
        std::cerr << "tried to set illegal size factor with value <= 0 (horizontalSizeFactor=" << horizontalSizeFactor << ", verticalSizeFactor=" << verticalSizeFactor << ")" << std::endl;
        return;
    }
    
    this->horizontalSizeFactor = std::min (horizontalSizeFactor, 1.0f);
    this->verticalSizeFactor = std::min (verticalSizeFactor, 1.0f);
    
    // Make sure we're still in bounds
    if (! isInBounds (centerPos))
        moveGlyphWithinBounds(); // if not, move the center so that we are in bounds
}

void Glyph::incrementVolume (float increment)
{
    this->volume = std::max (std::min (volume + increment, 1.0f), 0.0f);
}

void Glyph::incrementSizeFactor (float horizontalIncrement, float verticalIncrement)
{
//    this->sizeFactor = getSizeFactorWithinBounds (increment);
    this->horizontalSizeFactor = std::min (std::max (horizontalSizeFactor + horizontalIncrement, 0.05f), 1.0f);
    this->verticalSizeFactor = std::min (std::max (verticalSizeFactor + verticalIncrement, 0.05f), 1.0f);
    if (! isInBounds (centerPos))
        moveGlyphWithinBounds();
}

void Glyph::setCenterPos (juce::Point<float> centerPos)
{
    this->centerPos = centerPos;
    moveGlyphWithinBounds();
}

void Glyph::moveBy (std::pair<float, float> amountToMove)
{
    setCenterPos ({ centerPos.x + amountToMove.first, centerPos.y + amountToMove.second });
}

void Glyph::setIsCascading (bool isCascading)
{
    
}

void Glyph::setDensity (int density)
{
    
}

void Glyph::setStrokeOverlap (float strokeOverlap)
{
    
}

void Glyph::setDotOverlap (float dotOverlap)
{
    
}

bool Glyph::isInBounds (juce::Point<float> centerPos) const
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    bool isInXBounds = centerPos.x >= minX && centerPos.x <= maxX;
    bool isInYBounds = centerPos.y >= minY && centerPos.y <= maxY;
    return isInXBounds && isInYBounds;
}

void Glyph::moveGlyphWithinBounds()
{
    centerPos = getCenterPosWithinBounds (centerPos);
}

juce::Point<float> Glyph::getCenterPosWithinBounds (juce::Point<float> hypotheticalCenterPos) const
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    juce::Point<float> centerPosWithinBounds = hypotheticalCenterPos;
    centerPosWithinBounds.x = std::min (std::max (centerPosWithinBounds.x, minX), maxX);
    centerPosWithinBounds.y = std::min (std::max (centerPosWithinBounds.y, minY), maxY);
    
    return centerPosWithinBounds;
}

float Glyph::getSizeFactorWithinBounds (float hypotheticalIncrement) const
{
    float projectedSizeFactor = std::max (horizontalSizeFactor, verticalSizeFactor) + hypotheticalIncrement;
    float maxXSizeFactor = std::min (std::abs (-1.0f - centerPos.x), std::abs (1.0f - centerPos.x));
    float maxYSizeFactor = std::min (std::abs (-1.0f - centerPos.y), std::abs (1.0f - centerPos.y));
    projectedSizeFactor = std::min (projectedSizeFactor, std::min (maxXSizeFactor / width, maxYSizeFactor / height));
    
    return std::min (std::max (projectedSizeFactor, 0.02f), 1.0f);
}

std::pair<float, float> Glyph::getProjectedMoveDistance (juce::Point<float> hypotheticalCenterPos) const
{
    auto projectedEndPos = getCenterPosWithinBounds (hypotheticalCenterPos);
    
    return { projectedEndPos.x - centerPos.x, projectedEndPos.y - centerPos.y };
}

float Glyph::getProjectedSizeFactorIncrement (float increment) const
{
    return getSizeFactorWithinBounds (increment) - std::max (horizontalSizeFactor, verticalSizeFactor);
}

void Glyph::moveSizeFactorWithinBounds()
{
    float maxXSizeFactor = std::min (std::abs (-1.0f - centerPos.x), std::abs (1.0f - centerPos.x));
    float maxYSizeFactor = std::min (std::abs (-1.0f - centerPos.y), std::abs (1.0f - centerPos.y));
    float projectedHorizontalSizeFactor = std::min (horizontalSizeFactor, maxXSizeFactor / width);
    float projectedVerticalSizeFactor = std::min (verticalSizeFactor, maxYSizeFactor / height);
    float minScalar = std::min (projectedHorizontalSizeFactor / horizontalSizeFactor, projectedVerticalSizeFactor / verticalSizeFactor); // limit resize by biggest dimension.
    horizontalSizeFactor *= minScalar;
    verticalSizeFactor *= minScalar;
}

std::pair<float, float> Glyph::getXBounds() const
{
    float minX = -1.0f + horizontalSizeFactor * width;
    float maxX = 1.0f - horizontalSizeFactor * width;
    return { minX, maxX };
}

std::pair<float, float> Glyph::getYBounds() const
{
    float minY = -1.0f + verticalSizeFactor * height;
    float maxY = 1.0f - verticalSizeFactor * height;
    return { minY, maxY };
}

