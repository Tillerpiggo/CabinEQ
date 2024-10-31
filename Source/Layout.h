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
    Layout (juce::Rectangle<float> bounds); // implicitly starts as a rectangle that takes up the full area
    
    void setBounds (std::vector<juce::Component>& components);
    
    // Multiple rectangles in a row or column
    void addRowWithEvenlySpacedRects (int numRects);
    void addRowWithRectWidths (std::vector<FlexibleLayoutDimension> rectWidths);
    
    juce::Rectangle<float> getRectAtRow (int rowIdx, int rectIdx); // rect idx is the index of the rect in the row, from left to right
    
private:
    std::pair<float, float> getWidthRange();
    std::vector<std::pair<float, float>> getHeightRanges();
    
    juce::Rectangle<float> bounds; // the total bounds of this layout
    std::vector<Row> rows;
    
    std::vector<std::pair<float, float>> heightRanges;
    bool heightRangesAreUpdated = false;
};
