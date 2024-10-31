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
    
    void setBoundsOfComponents (std::vector<juce::Component*> components);
    void layoutComponentsInGrid (std::vector<std::vector<juce::Component*>> components); // creates an evenly spaced grid to layout the components matching the 2D vector input, and sets the bounds of the components
    void setPadding (float padding);
    
    // Multiple rectangles in a row or column
    void addRowWithEvenlySpacedRects (int numRects, FlexibleLayoutDimension height = FlexibleLayoutDimension::fill());
    void addRowWithRectWidths (std::vector<FlexibleLayoutDimension> rectWidths, FlexibleLayoutDimension height = FlexibleLayoutDimension::fill());
    
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
