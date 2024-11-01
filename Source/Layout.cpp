/*
  ==============================================================================

    Layout.cpp
    Created: 30 Oct 2024 7:14:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Layout.h"

Layout::Layout (juce::Rectangle<int> bounds, float padding)
    : bounds (bounds), rows ({}), padding (padding)
{
    
}

void Layout::setBoundsOfComponents (std::vector<juce::Component*> components)
{
    for (int componentIdx = 0; componentIdx < components.size(); ++componentIdx)
    {
        auto [rowIdx, rectIdx] = getRowAndRectIdx (componentIdx);
        auto bounds = getBoundsAt (rowIdx, rectIdx);
        components[componentIdx]->setBounds (bounds);
//        
//        std::cout << "Bounds(x: " << bounds.getX() << ", y: " << bounds.getY() << ", width: " << bounds.getWidth() << ", height: " << bounds.getHeight() << ")" << std::endl;
    }
}

void Layout::layoutComponentsInGrid (std::vector<std::vector<juce::Component*>> components)
{
    for (const auto& componentRow : components)
        addRowWithEvenlySpacedRects (static_cast<int> (componentRow.size()));
    
    for (int rowIdx = 0; rowIdx < components.size(); ++rowIdx)
        for (int rectIdx = 0; rectIdx < components[rowIdx].size(); ++rectIdx)
            components[rowIdx][rectIdx]->setBounds (getBoundsAt (rowIdx, rectIdx));
}

void Layout::setPadding (float padding)
{
    this->padding = padding;
    heightRangesAreUpdated = false;
}

void Layout::addRowWithEvenlySpacedRects (int numRects, FlexibleLayoutDimension height)
{
    rows.push_back (Row (getWidthRange(), height)
                        .withRectWidths(std::vector<FlexibleLayoutDimension> (numRects, FlexibleLayoutDimension::fill())));
    heightRangesAreUpdated = false;
}

void Layout::addRowWithRectWidths (std::vector<FlexibleLayoutDimension> rectWidths, FlexibleLayoutDimension height)
{
    rows.push_back (Row (getWidthRange(), height)
                        .withRectWidths (rectWidths));
    heightRangesAreUpdated = false;
}

juce::Rectangle<int> Layout::getBoundsAt (int rowIdx, int rectIdx)
{
    auto widthRange = rows[rowIdx].getWidthRanges()[rectIdx];
    auto heightRange = getHeightRanges()[rowIdx];
    return juce::Rectangle<int> (widthRange.first, heightRange.first - bounds.getY(), widthRange.second - widthRange.first, heightRange.second - heightRange.first);
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
        int numRectsInRow = rows[rowIdx].getNumRects();

        if (componentIdx < runningTotal + numRectsInRow)
        {
            int rectIdx = componentIdx - runningTotal;
            return { rowIdx, rectIdx };
        }

        runningTotal += numRectsInRow;
    }

    // If componentIdx is out of range, return invalid indices
    return { -1, -1 };
}
