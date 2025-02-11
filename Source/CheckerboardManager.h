/*
  ==============================================================================

    CheckerboardManager.h
    Created: 10 Feb 2025 7:31:36pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Checkerboard.h"

// A super simple class that, for now, is essentially just a wrapper on a Checkerboard. I might use it for more complex logic later
class CheckerboardManager
{
public:
    CheckerboardManager(); // initializes a default checkerboard
    
    const Checkerboard getCheckerboard();
    
    void setResolution (int resolution);
    void togglePolarity();
    
private:
    Checkerboard checkerboard;
};
