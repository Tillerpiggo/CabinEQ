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
    updateNoiseSources();
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

std::vector<NoisePoint> ArchetypalGlyph::cascadingPositionsAtTime (float time) const
{
    if (! isCascading)
        return { positionAtTime (time) };
    
    if (time < 0 || time >= 1)
    {
        std::cerr << "Called cascadingPositionAtTime in Glyph with invalid time outside of [0, 1). (time=" << time << ")" << std::endl;
    }
    
    std::vector<NoisePoint> cascadingPositionsAtTime;
    for (const auto& noiseSource : noiseSources)
    {
        auto noisePoint = noiseSource.noisePointAtTime (time);
        cascadingPositionsAtTime.push_back (noisePoint);
    }
    
    return cascadingPositionsAtTime;
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

const int ArchetypalGlyph::getNumNoiseSources() const
{
    if (! isCascading)
        return 1.0f;
    return (int) noiseSources.size();
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
    std::cout << "got this far" << std::endl;
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
    std::vector<juce::Point<float>> noisePositions;
    for (const auto& stroke : strokes)
    {
        int numPositions = std::ceil (stroke.getLength() * maxDensity);
        if (stroke.getLength() == 0)
            numPositions = 1;
        for (int i = 0; i < numPositions; ++i)
        {
            float t = (float) i / ((float) numPositions - 1.0f); // time relative to stroke so that the endpoints have points on them. TODO: modify to divide by numPositions if the stroke loops
            auto noisePos = stroke.positionAtTime (t);
            noisePositions.push_back (noisePos.point());
        }
    }
    
    // Figure out the start and end times for each stroke - stroke by stroke
    std::vector<std::pair<float, float>> noiseStartEndTimes;
    for (int strokeIdx = 0; strokeIdx < strokes.size(); strokeIdx++)
    {
        int numPositions = std::ceil (strokes[strokeIdx].getLength() * maxDensity);
        if (strokes[strokeIdx].getLength() == 0)
            numPositions = 1;
        
        float strokeStart = (float) strokeIdx / (float) (strokes.size());// + 1.0f);
        float minStrokeLength = 1.0f / (float) (strokes.size());// + 1.0f);
        float strokeLength = minStrokeLength + (1.0f - minStrokeLength) * strokeOverlap;
        float strokeEnd = strokeStart + strokeLength;
        for (int i = 0; i < numPositions; ++i)
        {
            float noiseStart = (float) i / (float) (numPositions);// + 1);
            float minNoiseLength = 1.0f / (float) (numPositions);// + 1);
            float noiseLength = minNoiseLength + (1.0f - minNoiseLength) * dotOverlap;
            float noiseEnd = noiseStart + noiseLength;
            float normalizedNoiseStart = noiseStart * (strokeEnd - strokeStart) + strokeStart;
            float normalizedNoiseEnd = noiseEnd * (strokeEnd - strokeStart) + strokeStart;
            
            noiseStartEndTimes.push_back ({ normalizedNoiseStart, normalizedNoiseEnd });
        }
    }
    
    for (int i = 0; i < noisePositions.size(); ++i)
    {
        auto pos = noisePositions[i];
        auto startEnd = noiseStartEndTimes[i];
        noiseSources.push_back ({ pos.x, pos.y, startEnd.first, startEnd.second, rampLength });
    }
}
