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
    void addRowWithRectWidths (std::vector<FLD> rectWidths);
    
    juce::Rectangle getRectAtRow (int rowIdx, int rectIdx); // rect idx is the index of the rect in the row, from left to right
    
private:
    juce::Rectangle<float> bounds; // the total bounds of this layout
    std::vector<Row> rows;
};
