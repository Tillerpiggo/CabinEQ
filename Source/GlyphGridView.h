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
    
    void updateIsPlaying(); // signals isPlaying is changed, triggers update of instruction text
    
private:
    void drawGridLines (juce::Graphics& g);
    void drawGlyphs (juce::Graphics& g);
    void drawCenterDots (juce::Graphics& g); // draws the center dots for the glyphs
    void drawPlayingDots (juce::Graphics& g);
    void drawGlyph (juce::Graphics& g, const std::vector<Stroke>& strokes, juce::Point<float> centerPos, std::pair<float, float> sizeFactor, juce::Colour strokeColour);
    void drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour);
    void drawDraggingGlyph (juce::Graphics& g);
    void drawSelection (juce::Graphics& g);
    juce::Point<float> getLocalPointFromNormalizedPoint (juce::Point<float> point, juce::Point<float> centerPos, std::pair<float, float> sizeFactor);
    juce::Point<float> getNormalizedPointFromLocalPoint (juce::Point<float> point);
    juce::Point<float> getLocalCenterPosForGlyph (const Glyph& glyph); // gets the local coords for the glyph's center point
    juce::Point<float> getNormalizedPointFromMouseEvent (const juce::MouseEvent& event);
    
    void updateHoveringStatus (const juce::MouseEvent& event);
    
    void dropDraggingGlyph(); // drops and adds the current dragging glyph, updates the glyphs, and resets the dragging variables
    void moveGlyph (int glyphId, juce::Point<float> centerPos);
    void moveSelectedGlyphsToMouseEvent (const juce::MouseEvent& event); // tries to move the selected glyphs to the given mouse event, assuming the event is a drag from the original starting position.
    void removeGlyph (int glyphId);
    void incrementVolume (int glyphId, float increment);
    void incrementSizeFactor (int glyphId, float increment);
    void incrementVerticalSizeFactor (int glyphId, float verticalIncrement);
    void scaleSelectedGlyphs (float increment);
    
    GlyphViewListener* listener = nullptr;
    GlyphViewDataSource* dataSource = nullptr;
    
    std::vector<Glyph> glyphs;
    
    float STROKE_WIDTH = 3.0f;
    juce::Colour STROKE_COLOUR = juce::Colours::teal;
    juce::Colour DOT_COLOUR = juce::Colours::turquoise;
    juce::Colour PLAYING_DOT_COLOUR = juce::Colours::lightgreen;
    juce::Colour BORDER_COLOUR = juce::Colours::teal;
    float DOT_RADIUS_DEFAULT = 6.0f;
    float DOT_RADIUS_DRAGGING = 8.0f;
    float DOT_RADIUS_PLAYING = 6.0f;
    float HOVER_MIN_DIST = 20.0f;
    
    // Drag and drop
    std::optional<ArchetypalGlyph> draggingGlyph;
    std::optional<juce::Point<float>> draggingPos;
    std::optional<float> draggingSize;
    bool isDraggingDuplicate = false;
    
    // Dragging glyphs
    int draggingId = -1;
    int hoveringId = -1;
    
    // Selection
    std::optional<juce::Point<float>> selectionStart;
    std::optional<juce::Point<float>> selectionEnd;
    std::optional<juce::Rectangle<float>> selectionRect;
    std::unordered_set<int> selectedIds;
    
    // Selection drag
    juce::Point<float> selectionStartPos; // the initial position you select
    std::unordered_map<int, juce::Point<float>> selectedIdToStartingPosition; // the starting position for each selected glyph, in local coordinates
    
    // Instruction text
    juce::Label instructionText;
    std::string dragInstructions { "Drag in a symbol to play noise" };
    std::string playInstructions { "Press play to play noise" };
    std::string pauseInstructions { "Press pause to stop noise" };
    std::string selectInstructions { "Drag to select multiple symbols" };
    std::string scrollInstructions { "Scroll to resize symbol" };
};
