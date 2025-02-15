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
    Checkerboard (std::string title, int numRows, int numCols, bool polarity = true);
    
    std::string getTitle() const;
    std::pair<int, int> getGridDimensions() const; // returns [rows, cols]
    bool getPolarity() const;
    int getNumNoiseGenerators() const; // calculates and returns the number of noise generators needed given this polarity and the resolution
    std::vector<std::vector<bool>> getGrid() const;
    
    void togglePolarity();
    
private:
    void calculateGrid();
    
    std::string title;
    int numRows;
    int numCols;
    bool polarity; // true = bottom left corner (0, 0) is filled, false = bottom left corner (0, 0) is empty
    
    std::vector<std::vector<bool>> grid; // [resolution x resolution] grid where 0 = off and 1 = on
};
