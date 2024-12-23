/*
  ==============================================================================

    GlyphGridView.h
    Created: 22 Dec 2024 11:00:52pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Listeners.h"
#include "ArchetypeView.h"

// This view displays multiple glyphs, that are attached to a grid, as they animate (in sync with the sounds they correspond with/produce)
class GlyphGridView  : public juce::Component,
                       public juce::DragAndDropTarget
{
public:
    GlyphGridView();
    ~GlyphGridView();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (GlyphViewListener* listener);
    void setDataSource (GlyphViewDataSource* dataSource);
    
    // Drag and drop methods
    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override;
    void itemDragEnter (const SourceDetails& dragSourceDetails) override;
    void itemDragMove (const SourceDetails& dragSourceDetails) override;
    void itemDragExit (const SourceDetails& dragSourceDetails) override;
    void itemDropped (const SourceDetails& dragSourceDetails) override;
    bool shouldDrawDragImageWhenOver() override;
    
private:
    void drawGlyphs (juce::Graphics& g);
    void drawGlyph (juce::Graphics& g, const std::vector<Stroke>& strokes, juce::Point<float> centerPos, float sizeFactor, juce::Colour strokeColour);
    void drawDraggingGlyph (juce::Graphics& g);
    juce::Point<float> getLocalPointFromNormalizedPoint (juce::Point<float> point, juce::Point<float> centerPos, float sizeFactor);
    juce::Point<float> getNormalizedPointFromLocalPoint (juce::Point<float> point);
    
    GlyphViewListener* listener = nullptr;
    GlyphViewDataSource* dataSource = nullptr;
    
    std::vector<Glyph> glyphs;
    
    float STROKE_WIDTH = 3.0f;
    juce::Colour STROKE_COLOUR = juce::Colours::pink;
    
    // Drag and drop
    std::optional<ArchetypalGlyph> draggingGlyph;
    std::optional<juce::Point<float>> draggingPos;
};
