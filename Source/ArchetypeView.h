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
    
    ArchetypalGlyph getArchetype();
    void setArchetype (ArchetypalGlyph archetype);
    
    void mouseDrag (const juce::MouseEvent &event) override;
    
private:
    juce::Point<float> normalizePointInBounds (juce::Point<float> point);
    
    ArchetypalGlyph archetype;
    
    float STROKE_WIDTH = 3.0f;
    juce::Colour STROKE_COLOUR = juce::Colours::lightblue;
};