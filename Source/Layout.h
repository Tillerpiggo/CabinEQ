/*
  ==============================================================================

    Layout.h
    Created: 30 Oct 2024 7:14:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Row.h"

// This class makes it easy to create evenly or proportionally spaced 2D interfaces by defining their bounds
class Layout
{
public:
    Layout (juce::Rectangle<int> bounds, float padding = 0.0f); // implicitly starts as a rectangle that takes up the full area
    
    void setBoundsOfComponents (std::vector<juce::Component*>& components);
    void setPadding (float padding);
    
    // Multiple rectangles in a row or column
    void addRowWithEvenlySpacedRects (int numRects);
    void addRowWithRectWidths (std::vector<FlexibleLayoutDimension> rectWidths);
    
    juce::Rectangle<int> getBoundsAt (int rowIdx, int rectIdx); // rect idx is the index of the rect in the row, from left to right. Returns the bounds relative to the parent component (i.e. relative to the top corner of bounds)
    
private:
    std::pair<float, float> getWidthRange();
    std::vector<std::pair<float, float>> getHeightRanges();
    std::vector<FlexibleLayoutDimension> getRowHeights();
    std::pair<int, int> getRowAndRectIdx (int componentIdx);
    
    juce::Rectangle<int> bounds; // the total bounds of this layout
    std::vector<Row> rows;
    float padding = 0.0f;
    
    std::vector<std::pair<float, float>> heightRanges;
    bool heightRangesAreUpdated = false;
};
