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
    
    void togglePolarity();
    
private:
    int resolution;
    bool polarity;
};
