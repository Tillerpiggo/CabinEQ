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
    Checkerboard (int resolution, bool polarity);
    
    int getResolution();
    bool getPolarity();
    int getNumNoiseGenerators(); // returns the number of noise generators needed given this polarity and this time
    
    void togglePolarity();
    
private:
    int resolution;
    bool polarity; // true = bottom left corner (0, 0) is filled, false = bottom left corner (0, 0) is empty
};
