/*
  ==============================================================================

    ArchetypalGlyph.cpp
    Created: 8 Feb 2025 2:05:06pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ArchetypalGlyph.h"

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

bool ArchetypalGlyph::getIsCascading() const
{
    return isCascading;
}

int ArchetypalGlyph::getDensity() const
{
    return density;
}

float ArchetypalGlyph::getStrokeOverlap() const
{
    return strokeOverlap;
}

float ArchetypalGlyph::getDotOverlap() const
{
    return dotOverlap;
}

float ArchetypalGlyph::getRampLength() const
{
    return rampLength;
}

void ArchetypalGlyph::setCascadeSettings (bool isCascading, int density, float strokeOverlap, float dotOverlap, float rampLength)
{
    this->isCascading = isCascading;
    this->density = density;
    this->strokeOverlap = strokeOverlap;
    this->dotOverlap = dotOverlap;
    this->rampLength = rampLength;
    updateNoiseSources();
}

void ArchetypalGlyph::updateNoiseSources()
{
    // Recalculate noise sources from settings
    noiseSources.clear();
    
    // Noise sources is only used for cascading calculations anyways
    if (! isCascading)
        return;
    
    // Get the max density of noise sources for each stroke per unit length
    Stroke longestStroke = strokes[0];
    for (const auto& stroke : strokes)
    {
        if (stroke.getLength() > longestStroke.getLength())
            longestStroke = stroke;
    }
    float maxDensity = density / longestStroke.getLength();
    
    // Figure out the noise source positions for each stroke - stroke by stroke
    
}
