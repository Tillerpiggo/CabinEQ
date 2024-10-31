/*
  ==============================================================================

    Layout.cpp
    Created: 30 Oct 2024 7:14:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Layout.h"

Layout::Layout (juce::Rectangle<float> bounds, float padding)
    : bounds (bounds), rows ({}), padding (padding)
{
    
}

void Layout::setBoundsOfComponents (std::vector<juce::Component>& components)
{
    for (int componentIdx = 0; componentIdx < components.size(); ++componentIdx)
    {
        auto [rowIdx, rectIdx] = getRowAndRectIdx (componentIdx);
        components[componentIdx].setBounds (getRectAtRow (rowIdx, rectIdx));
    }
}

void Layout::setPadding (float padding)
{
    this->padding = padding;
    heightRangesAreUpdated = false;
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

juce::Rectangle<int> Layout::getRectAtRow (int rowIdx, int rectIdx)
{
    auto widthRange = rows[rowIdx].getWidthRanges()[rectIdx];
    auto heightRange = getHeightRanges()[rowIdx];
    return juce::Rectangle<int> (widthRange.first, heightRange.first, widthRange.second, heightRange.second);
}

std::pair<float, float> Layout::getWidthRange()
{
    return { bounds.getX(), bounds.getX() + bounds.getWidth() };
}

std::vector<std::pair<float, float>> Layout::getHeightRanges()
{
    if (heightRangesAreUpdated)
        return heightRanges;
    
    std::pair<float, float> heightRange = { bounds.getY(), bounds.getY() + bounds.getHeight() };
    heightRanges = FlexibleLayoutDimension::lengthRangesForFlexibleLayoutDimensions (getRowHeights(), heightRange, padding);
    heightRangesAreUpdated = true;
    
    return heightRanges;
}

std::vector<FlexibleLayoutDimension> Layout::getRowHeights()
{
    std::vector<FlexibleLayoutDimension> rowHeights;
    for (const auto& row : rows)
        rowHeights.push_back (row.getHeight());
    return rowHeights;
}

std::pair<int, int> Layout::getRowAndRectIdx (int componentIdx)
{
    int runningTotal = 0;
    for (int rowIdx = 0; rowIdx < rows.size(); ++rowIdx)
    {
        const Row& row = rows[rowIdx];
        int numRectsInRow = row.getNumRects();

        if (componentIdx < runningTotal + numRectsInRow)
        {
            int rectIdx = componentIdx - runningTotal;
            return {rowIdx, rectIdx};
        }

        runningTotal += numRectsInRow;
    }

    // If componentIdx is out of range, return invalid indices
    return { -1, -1 };
}
