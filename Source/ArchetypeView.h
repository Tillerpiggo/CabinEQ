/*
  ==============================================================================

    ArchetypeView.h
    Created: 22 Dec 2024 9:07:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"

// This draws a single archetypal glyph
class ArchetypeView  : public juce::Component
{
public:
    ArchetypeView (ArchetypalGlyph archetype);
    ~ArchetypeView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setArchetype (ArchetypalGlyph archetype);
    
private:
    juce::Point<float> normalizePointInBounds (juce::Point<float> point);
    
    std::optional<ArchetypalGlyph> archetype;
    
    float STROKE_WIDTH = 3.0f;
    juce::Colour STROKE_COLOUR = juce::Colours::lightblue;
};
