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
                       public juce::DragAndDropTarget,
                       public juce::Timer
{
public:
    GlyphGridView();
    ~GlyphGridView();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (GlyphViewListener* listener);
    void setDataSource (GlyphViewDataSource* dataSource);
    
    void timerCallback() override;
    
    // Mouse events
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    // Drag and drop methods
    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override;
    void itemDragEnter (const SourceDetails& dragSourceDetails) override;
    void itemDragMove (const SourceDetails& dragSourceDetails) override;
    void itemDragExit (const SourceDetails& dragSourceDetails) override;
    void itemDropped (const SourceDetails& dragSourceDetails) override;
    bool shouldDrawDragImageWhenOver() override;
    
private:
    void drawGlyphs (juce::Graphics& g);
    void drawCenterDots (juce::Graphics& g); // draws the center dots for the glyphs
    void drawGlyph (juce::Graphics& g, const std::vector<Stroke>& strokes, juce::Point<float> centerPos, float sizeFactor, juce::Colour strokeColour);
    void drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour);
    void drawDraggingGlyph (juce::Graphics& g);
    juce::Point<float> getLocalPointFromNormalizedPoint (juce::Point<float> point, juce::Point<float> centerPos, float sizeFactor);
    juce::Point<float> getNormalizedPointFromLocalPoint (juce::Point<float> point);
    juce::Point<float> getLocalizedCenterPointForGlyph (const Glyph& glyph); // gets the local coords for the glyph's center point
    juce::Point<float> getNormalizedPointFromMouseEvent (const juce::MouseEvent& event);
    
    void updateHoveringStatus (const juce::MouseEvent& event);
    
    void moveGlyph (int glyphId, juce::Point<float> centerPos);
    void removeGlyph (int glyphId);
    
    GlyphViewListener* listener = nullptr;
    GlyphViewDataSource* dataSource = nullptr;
    
    std::vector<Glyph> glyphs;
    
    float STROKE_WIDTH = 3.0f;
    juce::Colour STROKE_COLOUR = juce::Colours::pink;
    juce::Colour DOT_COLOUR = juce::Colours::turquoise;
    float DOT_RADIUS_DEFAULT = 6.0f;
    float DOT_RADIUS_DRAGGING = 8.0f;
    float HOVER_MIN_DIST = 30.0f;
    
    // Drag and drop
    std::optional<ArchetypalGlyph> draggingGlyph;
    std::optional<juce::Point<float>> draggingPos;
    
    // Dragging glyphs
    int draggingId = -1;
    int hoveringId = -1;
};
