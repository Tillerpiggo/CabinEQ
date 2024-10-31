/*
  ==============================================================================

    Layout.h
    Created: 30 Oct 2024 7:14:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class FlexibleLayoutDimension; // forward declaration because I like the order of things in this file

// This class makes it easy to create evenly or proportionally spaced 2D interfaces by defining their bounds
class Layout
{
public:
    Layout (juce::Rectangle<float> bounds); // implicitly starts as a rectangle that takes up the full area
    
    std::vector<juce::Rectangle<float>> getRects(); // returns the current list of rectangles/bounds, in the order they were added
    
    // Adds a rect such that it takes half of the current space
    void addRectAbove (float padding = 0.0f, float newRectProportion = 0.5f);
    void addRectBelow (float padding = 0.0f, float newRectProportion = 0.5);
    void addRectLeft (float padding = 0.0f, float newRectProportion = 0.5);
    void addRectRight (float padding = 0.0f, float newRectProportion = 0.5);
    
    // Multiple rectangles in a row or column
    void addRowAbove (int numNewRects, float inBetweenPadding, float padding = 0.0f, float newRectProportion = 0.5f);
    
private:
    juce::Rectangle<float> bounds; // the total bounds of this layout
    std::vector<juce::Rectangle<float>> rects;
};

// This class manages a single row of rects, including the proportion of the rects in the row, and the desired height of the row itself. For now, row heights will always be evenly distributed within a layout.
class Row
{
public:
    Row(); // implicitly starts as a single rect that takes up the entire row, with height that is evenly distributed
    
    void addRectLeft();
    void addRectRight();
    void addRectLeft (float width);
    void addRectRight (float )
    void setPadding (float padding); // sets internal padding
    
private:
    std::vector<FlexibleLayoutDimension> rectWidths;
    float padding;
};

// This class represents a dimension (width or height) that can be flexible and expand to fill the designated area, or have a fixed size
class FlexibleLayoutDimension
{
public:
    enum class Type
    {
        fill, // fills available area
        proportional, // fills fixed percent of available area
        fixed // absolute value of dimension
    };
    
    static FlexibleLayoutDimension fill()
    {
        return FlexibleLayoutDimension (Type::fill, std::nullopt, std::nullopt);
    }
    
    static FlexibleLayoutDimension proportional (float proportionalValue)
    {
        return FlexibleLayoutDimension (Type::proportional, std::nullopt, proportionalValue);
    }
    
    static FlexibleLayoutDimension fixed (float fixedValue)
    {
        return FlexibleLayoutDimension (Type::fixed, std::nullopt, fixedValue);
    }
    
    
    
    const Type& getType() const
    {
        return type;
    }
    
    const float getFixedValue() const
    {
        return fixedValue.value(); // will crash if this class is not fixed
    }
    
    const float getProportionalValue() const
    {
        return proprotionalValue.value(); // will crash if this class is not relative
    }
    
private:
    FlexibleLayoutDimension (Type type, std::optional<float> fixedValue, std::optional<float> proportionalValue)
        : type (type), fixedValue (fixedValue), proportionalValue (proportionalValue)
    {}
    
    Type type;
    std::optional<float> fixedValue;
    std::optional<float> proportionalValue;
};
