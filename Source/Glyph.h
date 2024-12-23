/*
  ==============================================================================

    Glyph.h
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Stroke.h"

// This represents a Glyph archetype that can be copied, which is simply a sequence of strokes. It can tell you the needed position at a given time
class ArchetypalGlyph
{
public:
    ArchetypalGlyph (int id, std::vector<Stroke> initialStrokes = {});
    std::pair<juce::Point<float>, float> positionAtTime (float time) const; // time from [0, 1). Returns (pos, %) pair
    const std::vector<Stroke>& getStrokes() const;
    const std::vector<juce::Point<float>> getVertices() const;
    const int getId() const;
    
private:
    int id;
    std::vector<Stroke> strokes;
};

// This represents an actual Glyph that has a position, size, and potentially more state
class Glyph
{
public:
    Glyph (int id, ArchetypalGlyph archetype);
    Glyph (int id, ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor);
    
    std::pair<juce::Point<float>, float> positionAtTime (float time) const;
    const std::vector<Stroke>& getStrokes() const;
    const std::vector<juce::Point<float>> getVertices() const;
    
    float getSizeFactor() const;
    juce::Point<float> getCenterPos() const;
    int getId() const;
    
    void setSizeFactor (float sizeFactor);
    void setCenterPos (juce::Point<float> centerPos);
    
    bool isInBounds() const;
    void moveGlyphWithinBounds(); // changes centerPos so that glyph is still in bounds
    std::pair<float, float> getXBounds() const; // returns min x and max x for the current size
    std::pair<float, float> getYBounds() const; // returns min y and max y for the current size
    
private:
    int id;
    ArchetypalGlyph archetype;
    float sizeFactor = 1.0f; // can be in (0, 1], with 1 being full-sized
    juce::Point<float> centerPos = { 0.0f, 0.0f }; // can be x in [-1, 1], y in [-1, 1]
};
