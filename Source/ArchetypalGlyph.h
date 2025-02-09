/*
  ==============================================================================

    ArchetypalGlyph.h
    Created: 8 Feb 2025 2:05:06pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "NoiseSource.h"
#include "Stroke.h"

// This represents a Glyph archetype that can be copied, which is simply a sequence of strokes. It can tell you the needed position at a given time
class ArchetypalGlyph
{
public:
    ArchetypalGlyph (int id, std::vector<Stroke> initialStrokes = {});
    NoisePoint positionAtTime (float time) const; // time from [0, 1)
    std::vector<NoisePoint> cascadingPositionsAtTime (float time) const; // time from [0, 1), gives list of multiple positions
    
    const std::vector<Stroke>& getStrokes() const;
    const std::vector<NoisePoint> getVertices() const;
    const int getNumNoiseSources() const;
    const int getId() const;
    bool getIsCascading() const;
    int getDensity() const;
    float getStrokeOverlap() const;
    float getDotOverlap() const;
    float getRampLength() const;
    
    void setCascadeSettings (bool isCascading, int density, float strokeOverlap, float dotOverlap, float rampLength);
    
private:
    void updateNoiseSources();
    
    int id;
    std::vector<Stroke> strokes;
    
    std::vector<NoiseSource> noiseSources;
    bool isCascading = false;
    int density = 5; // # noise sources in longest stroke
    float strokeOverlap = 0.0; // % overlap between strokes
    float dotOverlap = 0.2; // % overlap between dots within stroke
    float rampLength = 0.5; // % length of ramp compared to overall length
};
