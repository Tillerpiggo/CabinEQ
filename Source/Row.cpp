/*
  ==============================================================================

    Row.cpp
    Created: 30 Oct 2024 8:13:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Row.h"


Row::Row (std::pair<float, float> widthRange, FlexibleLayoutDimension height, float padding)
    : height (height), rectWidths ({ FlexibleLayoutDimension::fill() }), widthRange (widthRange), padding (padding)
{
}

Row::Row (std::pair<float, float> widthRange, FlexibleLayoutDimension height, std::vector<FlexibleLayoutDimension> rectWidths, float padding)
    : height (height), rectWidths (rectWidths), widthRange (widthRange), padding (padding)
{
}

Row Row::withRectWidths (std::vector<FlexibleLayoutDimension> rectWidths)
{
    return Row (widthRange, height, rectWidths, padding);
}

void Row::setPadding (float padding)
{
    this->padding = padding;
    widthRangesAreUpdated = false;
}

std::vector<std::pair<float, float>> Row::getWidthRanges()
{
    if (widthRangesAreUpdated)
        return widthRanges;
    
    widthRanges = FlexibleLayoutDimension::lengthRangesForFlexibleLayoutDimensions (rectWidths, widthRange, padding);
    widthRangesAreUpdated = true;
    
    return widthRanges;
}

FlexibleLayoutDimension Row::getHeight() const
{
    return height;
}

int Row::getNumRects() const
{
    return rectWidths.size();
}
