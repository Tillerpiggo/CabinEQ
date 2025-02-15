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

// Manages a list of checkerboards sequentially and allows the user to change between them
class CheckerboardManager
{
public:
    CheckerboardManager(); // initializes a default checkerboard
    
    const Checkerboard getCurrCheckerboard();
    
    void goToNext();
    void goToPrev();
    bool hasNext();
    bool hasPrev();
    
    void togglePolarity(); // toggles polarity of the current checkerboard
    
private:
    std::vector<Checkerboard> checkerboards {{ 2, 2 }, { 3, 2 }, { 3, 3 }, { 4, 3 }, { 5, 3 }, { 6, 3 }, { 4, 4 }, { 5, 5 }, { 7, 3 }, { 8, 3 }};
    int currIdx = 0;
};
