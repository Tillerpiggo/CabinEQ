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
        return FlexibleLayoutDimension (Type::fixed, std::nullopt, std::nullopt);
    }
    
    static FlexibleLayoutDimension proportional (float proportionalValue)
    {
        return FlexibleLayoutDimension (Type::proportional, proportionalValue, std::nullopt);
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
    
private:
    FlexibleLayoutDimension (Type type, std::optional<float> fixedValue, std::optional<float> proportionalValue)
        : type (type), fixedValue (fixedValue), proportionalValue (proportionalValue)
    {}
    
    Type type;
    std::optional<float> fixedValue;
    std::optional<float> proportionalValue;
};
