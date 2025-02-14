/*
  ==============================================================================

    Checkerboard.h
    Created: 10 Feb 2025 3:15:01pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Checkerboard
{
public:
    Checkerboard();
    Checkerboard (int resolution, float sharpness, bool polarity);
    
    int getResolution() const;
    float getSharpness() const;
    bool getPolarity() const;
    int getNumNoiseGenerators() const; // calculates and returns the number of noise generators needed given this polarity and the resolution
    std::vector<std::vector<bool>> getGrid() const;
    
    void setResolution (int resolution);
    void setSharpness (float sharpness);
    void togglePolarity();
    
private:
    void calculateGrid();
    
    int resolution;
    float sharpness; // from 1.0f (least sharp) to 0.5f (most sharp)
    bool polarity; // true = bottom left corner (0, 0) is filled, false = bottom left corner (0, 0) is empty
    
    std::vector<std::vector<bool>> grid; // [resolution x resolution] grid where 0 = off and 1 = on
};
