/*
  ==============================================================================

    Glyph.cpp
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Glyph.h"

ArchetypalGlyph::ArchetypalGlyph (int id, std::vector<Stroke> initialStrokes)
    : id (id), strokes (initialStrokes)
{
}

std::pair<juce::Point<float>, float> ArchetypalGlyph::positionAtTime (float time) const
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

const std::vector<Stroke>& ArchetypalGlyph::getStrokes() const
{
    return strokes;
}

const std::vector<juce::Point<float>> ArchetypalGlyph::getVertices() const
{
    // this will overlap the end points... whatever for now
    std::vector<juce::Point<float>> vertices;
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

Glyph::Glyph (int id, ArchetypalGlyph archetype)
    : id (id), archetype (archetype)
{
}

Glyph::Glyph (int id, ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor)
    : id (id), archetype (archetype), sizeFactor (sizeFactor), centerPos (centerPos)
{
    std::cout << "centerPos: (x: " << centerPos.x << ", y: " << centerPos.y << ")" << std::endl;
    moveGlyphWithinBounds();
}

std::pair<juce::Point<float>, float> Glyph::positionAtTime (float time) const
{
    return archetype.positionAtTime (time);
}

const std::vector<Stroke>& Glyph::getStrokes() const
{
    return archetype.getStrokes();
}

const std::vector<juce::Point<float>> Glyph::getVertices() const
{
    return archetype.getVertices();
}

float Glyph::getSizeFactor() const
{
    return sizeFactor;
}

juce::Point<float> Glyph::getCenterPos() const
{
    return centerPos;
}

int Glyph::getId() const
{
    return id;
}

void Glyph::setSizeFactor (float sizeFactor)
{
    if (sizeFactor <= 0)
    {
        std::cerr << "tried to set illegal size factor with value <= 0 (sizeFactor=" << sizeFactor << ")" << std::endl;
        return;
    }
    
    this->sizeFactor = std::min (sizeFactor, 1.0f);
    
    // Make sure we're still in bounds
    if (! isInBounds())
        moveGlyphWithinBounds(); // if not, move the center so that we are in bounds
}

void Glyph::setCenterPos (juce::Point<float> centerPos)
{
    this->centerPos = centerPos;
    moveGlyphWithinBounds();
}

bool Glyph::isInBounds() const
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    juce::Point<float> centerPos = getCenterPos();
    
    bool isInXBounds = centerPos.x >= minX && centerPos.x <= maxX;
    bool isInYBounds = centerPos.y >= minY && centerPos.y <= maxY;
    return isInXBounds && isInYBounds;
}

void Glyph::moveGlyphWithinBounds()
{
    auto [minX, maxX] = getXBounds();
    auto [minY, maxY] = getYBounds();
    
    centerPos.x = std::min (std::max (centerPos.x, minX), maxX);
    centerPos.y = std::min (std::max (centerPos.y, minY), maxY);
}

std::pair<float, float> Glyph::getXBounds() const
{
    float minX = -1.0f + sizeFactor;
    float maxX = 1.0f - sizeFactor;
    return { minX, maxX };
}

std::pair<float, float> Glyph::getYBounds() const
{
    float minY = -1.0f + sizeFactor;
    float maxY = 1.0f - sizeFactor;
    return { minY, maxY };
}

