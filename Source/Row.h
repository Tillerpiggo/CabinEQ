/*
  ==============================================================================

    Row.h
    Created: 30 Oct 2024 8:13:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "FlexibleLayoutDimension.h"

// This class manages a single row of rects, including the proportion of the rects in the row, and the desired height of the row itself. For now, row heights will always be evenly distributed within a layout.
class Row
{
public:
    Row (std::pair<float, float> widthRange, FlexibleLayoutDimension height = FlexibleLayoutDimension::fill()); // implicitly starts as a single rect that takes up the entire row, with height that is evenly distributed
    
    Row (std::pair<float, float> widthRange, FlexibleLayoutDimension height, std::vector<FlexibleLayoutDimension> rectWidths, float padding);
    
    Row withRectWidths (std::vector<FlexibleLayoutDimension> widths);
    void setPadding (float padding); // sets internal padding
    
    std::vector<std::pair<float, float>> getWidthRanges(); // returns the width ranges, in order from left to right, of all the rectangles in this row
    FlexibleLayoutDimension getHeight() const;
    int getNumRects() const;
    
private:
    FlexibleLayoutDimension height;
    std::vector<FlexibleLayoutDimension> rectWidths;
    std::pair<float, float> widthRange;
    float padding;
    
    std::vector<std::pair<float, float>> widthRanges;
    bool widthRangesAreUpdated = false;
};

