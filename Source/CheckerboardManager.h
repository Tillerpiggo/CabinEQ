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
    int getNumCheckerboards();
    std::string getNameAtIdx (int idx);
    int getSelectedRow();
    
    void goToNext();
    void goToPrev();
    bool hasNext();
    bool hasPrev();
    
    void togglePolarity(); // toggles polarity of the current checkerboard
    void selectIdx (int idx);
    
private:
    std::vector<Checkerboard> checkerboards {{ "Calibration I", 2, 2 }, { "Calibration II", 3, 2 }, { "Calibration III", 3, 3 }, { "Calibration IV", 4, 3 }, { "Calibration V", 5, 3 }, { "Calibration VI", 6, 3 }, { "Calibration VII", 4, 4 }, { "Calibration VIII", 5, 5 }, { "Calibration IX", 7, 3 }, { "Calibration X", 8, 3 }, { "Calibration X", 20, 3 }};
    int currIdx = 0;
};
