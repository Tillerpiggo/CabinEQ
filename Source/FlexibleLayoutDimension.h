/*
  ==============================================================================

    FlexibleLayoutDimension.h
    Created: 30 Oct 2024 8:13:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a dimension (width or height) that can be flexible and expand to fill the designated area, or have a fixed size
class FlexibleLayoutDimension // FlexibleLayoutDimension
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
    
    static FlexibleLayoutDimension fixed (float fixedValue)
    {
        return FlexibleLayoutDimension (Type::fixed, fixedValue, std::nullopt);
    }
    
    static FlexibleLayoutDimension proportional (float proportionalValue)
    {
        return FlexibleLayoutDimension (Type::proportional, std::nullopt, proportionalValue);
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
        return proportionalValue.value(); // will crash if this class is not relative
    }
    
    // Gets the overall static value within a certain size
    const float getStaticValueIn (float totalSpace) const
    {
        switch (type)
        {
            case Type::fill:
                std::cerr << "Trying to get static value of fill type, which is undefined" << std::endl;
                break;
            case Type::fixed:
                return getFixedValue();
            case Type::proportional:
                return getProportionalValue() * totalSpace;
        }
    }
    
    bool isStatic() const
    {
        return type == Type::fixed || type == Type::proportional;
    }
    
    // Where absolute range is a pair of the lowest and highest values (e.g. the leftmost and rightmost points) that the flexible layout dimensions are living in
    static const std::vector<std::pair<float, float>> lengthRangesForFlexibleLayoutDimensions (std::vector<FlexibleLayoutDimension> lengths, std::pair<float, float> absoluteRange, float padding)
    {
        // First, calculate out absolute widths
        float availableLength = absoluteRange.second - absoluteRange.first - (lengths.size() + 1) * padding;
        
        // Allocate out static widths first (proportional and fixed)
        float staticLength = 0;
        for (const auto& length : lengths)
            if (length.isStatic())
                staticLength += length.getStaticValueIn (availableLength);
        
        if (staticLength > availableLength)
            std::cerr << "Trying to create lengthRanges with static widths exceeding total available space" << std::endl;
        
        // Calculate out how much width is remaining
        int numDynamicLengths = 0;
        for (const auto& length : lengths)
            if (! length.isStatic())
                numDynamicLengths++;
        
        float dynamicLength = (availableLength - staticLength) / static_cast<float> (numDynamicLengths);
        
        // Now calculate out all of the width ranges from left to right
        std::vector<std::pair<float, float>> lengthRanges;
        float currX = absoluteRange.first;
        currX += padding;
        for (const auto& length : lengths)
        {
            float startX = currX;
            float endX = currX;
            if (length.isStatic())
            {
                endX += length.getStaticValueIn (availableLength);
            }
            else
            {
                endX += dynamicLength;
            }
            lengthRanges.push_back ({ startX, endX });
            currX = endX;
            currX += padding;
        }
        
        return lengthRanges;
    }
    
private:
    FlexibleLayoutDimension (Type type, std::optional<float> fixedValue, std::optional<float> proportionalValue)
        : type (type), fixedValue (fixedValue), proportionalValue (proportionalValue)
    {}
    
    Type type;
    std::optional<float> fixedValue;
    std::optional<float> proportionalValue;
};
