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

// This represents a (componentPtr, flexibleLayoutDimension) pair, which is very helpful syntactically for defining easier to read user interfaces with Layout
class Space
{
public:
    Space()
        : componentPtr (nullptr), layoutDimension (FlexibleLayoutDimension::fill())
    {}
    
    Space (juce::Component* componentPtr)
        : componentPtr (componentPtr), layoutDimension (FlexibleLayoutDimension::fill())
    {}
    
    Space (juce::Component* componentPtr, float fixedSize)
        : componentPtr (componentPtr), layoutDimension (FlexibleLayoutDimension::fixed (fixedSize))
    {}
    
    Space (juce::Component* componentPtr, FlexibleLayoutDimension layoutDimension)
        : componentPtr (componentPtr), layoutDimension (layoutDimension)
    {}
    
    Space (float fixedSize)
        : componentPtr (nullptr), layoutDimension (FlexibleLayoutDimension::fixed (fixedSize))
    {}
    
    Space withProportionalSize (float proportionalSize)
    {
        return Space (componentPtr, FlexibleLayoutDimension::proportional (proportionalSize));
    }
    
    Space withFixedSize (float fixedSize)
    {
        return Space (componentPtr, FlexibleLayoutDimension::fixed (fixedSize));
    }
    
    juce::Component* getComponentPtr() const
    {
        return componentPtr;
    }
    
    FlexibleLayoutDimension getFlexibleLayoutDimension() const
    {
        return layoutDimension;
    }
    
private:
    juce::Component* componentPtr;
    FlexibleLayoutDimension layoutDimension;
};

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
    
    // Easily add rows more nicely
    void addRow (std::vector<Space> spaces, float height = -1); // adds row with fill height. Height being -1 = fill, (0, 1) = proportional, and 1+ = fixed
    
    juce::Rectangle<int> getBoundsAt (int rowIdx, int rectIdx); // rect idx is the index of the rect in the row, from left to right. Returns the bounds relative to the parent component (i.e. relative to the top corner of bounds)
    
    void updateComponentBounds(); // update the bounds of the components (which are added from addRow/addRowWithHeight/addRowWithProportionalHeight)
    
private:
    std::pair<float, float> getWidthRange();
    std::vector<std::pair<float, float>> getHeightRanges();
    std::vector<FlexibleLayoutDimension> getRowHeights();
    std::pair<int, int> getRowAndRectIdx (int componentIdx);
    std::vector<juce::Component*> components;
    
    juce::Rectangle<int> bounds; // the total bounds of this layout
    std::vector<Row> rows;
    float padding = 0.0f;
    
    std::vector<std::pair<float, float>> heightRanges;
    bool heightRangesAreUpdated = false;
};
