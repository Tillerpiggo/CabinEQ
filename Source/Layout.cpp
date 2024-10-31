/*
  ==============================================================================

    Layout.cpp
    Created: 30 Oct 2024 7:14:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Layout.h"

Layout::Layout (juce::Rectangle<float> bounds)
    : bounds (bounds), rows ({})
{
    
}

void Layout::setBounds (std::vector<juce::Component>& components)
{
    int componentIdx = 0;
    for (const auto& component : components)
    {
        // set component bounds with row and subrow idx
        componentIdx++;
    }
}

void Layout::addRowWithEvenlySpacedRects (int numRects)
{
    rows.push_back (Row (getWidthRange())
                        .withRectWidths(std::vector<FlexibleLayoutDimension> (numRects, FlexibleLayoutDimension::fill())));
}
void Layout::addRowWithRectWidths (std::vector<FlexibleLayoutDimension> rectWidths)
{
    rows.push_back (Row (getWidthRange())
                        .withRectWidths (rectWidths));
}

juce::Rectangle<float> Layout::getRectAtRow (int rowIdx, int rectIdx)
{
    auto widthRange = rows[rowIdx].getWidthRanges()[rectIdx];
    auto heightRange = getHeightRanges()[rowIdx];
    return juce::Rectangle<float> (widthRange.first, heightRange.first, widthRange.second, heightRange.second);
}

std::pair<float, float> Layout::getWidthRange()
{
    return { bounds.getX(), bounds.getX() + bounds.getWidth() };
}

std::vector<std::pair<float, float>> Layout::getHeightRanges()
{
    if (heightRangesAreUpdated)
        return heightRanges;
    
    heightRanges = FlexibleLayoutDimension::lengthRangesForFlexibleLayoutDimensions (rowHeights, widthRange, padding);
    heightRangesAreUpdated = true;
    
    return heightRanges;
}
