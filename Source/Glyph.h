/*
  ==============================================================================

    Glyph.h
    Created: 13 Nov 2024 10:35:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArchetypalGlyph.h"

// This represents an actual Glyph that has a position, size, and potentially more state
class Glyph
{
public:
    Glyph (int id, ArchetypalGlyph archetype);
    Glyph (int id, ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor);
    
    NoisePoint positionAtTime (float time) const;
    std::vector<NoisePoint> positionsAtTime (float time) const;
    const std::vector<Stroke>& getStrokes() const;
    const std::vector<NoisePoint> getVertices() const;
    const int getNumNoiseSources() const;
    ArchetypalGlyph getArchetype() const; // returns an archetype of this glyph
    
    std::pair<float, float> getSizeFactor() const;
    float getVolume() const;
    juce::Point<float> getCenterPos() const;
    int getId() const;
    
    void setSizeFactor (float horizontalSizeFactor, float verticalSizeFactor);
    void incrementVolume (float increment); // increments volume, with bounds 0 and 1
    void incrementSizeFactor (float horizontalIncrement, float verticalIncrement); // increments size factor, with bounds, and then updates surroundings.
    void setCenterPos (juce::Point<float> centerPos);
    void moveBy (std::pair<float, float> amountToMove); // moves by [moveX, moveY]
    void setIsCascading (bool isCascading);
    void setDensity (int density);
    void setStrokeOverlap (float strokeOverlap);
    void setDotOverlap (float dotOverlap);
    void setRampLength (float rampLength);
    
    bool isInBounds (juce::Point<float> centerPos) const; // returns if this glyph would still be in bounds if it had the given center position
    void moveGlyphWithinBounds(); // changes centerPos so that glyph is still in bounds
    juce::Point<float> getCenterPosWithinBounds (juce::Point<float> hypotheticalCenterPos) const; // gets a center position within bounds, given the hypothetical center position
    float getSizeFactorWithinBounds (float hypotheticalIncrement) const;
    std::pair<float, float> getProjectedMoveDistance (juce::Point<float> hypotheticalCenterPos) const; // returns the projected [moveX, moveY] if you were to try to move the glyph to this location
    float getProjectedSizeFactorIncrement (float increment) const;
    void moveSizeFactorWithinBounds(); // changes sizeFactor so that glyph is still in bounds
    std::pair<float, float> getXBounds() const; // returns min x and max x for the current size
    std::pair<float, float> getYBounds() const; // returns min y and max y for the current size
    
private:
    int id;
    ArchetypalGlyph archetype;
//    float sizeFactor = 1.0f; // can be in (0, 1], with 1 being full-sized
    float horizontalSizeFactor = 1.0f;
    float verticalSizeFactor = 1.0f;
    float volume = 1.0f; // can be from (0, 1], with 0 being probably around -20db or something
    float width = 1.0f;
    float height = 1.0f;
    juce::Point<float> centerPos = { 0.0f, 0.0f }; // can be x in [-1, 1], y in [-1, 1]
};
