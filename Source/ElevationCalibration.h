/*
  ==============================================================================

    ElevationCalibration.h
    Created: 23 Feb 2025 10:25:13am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

class ElevationCalibration
{
public:
    ElevationCalibration();
    
    // Row management
    bool canIncrementRows() const;
    bool canDecrementRows() const;
    void incrementRows();
    void decrementRows();
    int getNumRows() const { return numRows; }
    
    // Row selection
    void setSelectedRow(int row);
    int getSelectedRow() const { return selectedRow; }
    
    // Playback
    int getPlayingRowAtTime(double normalizedTime) const;
    
private:
    std::vector<int> getAdjacentRows(int row) const;
    std::vector<int> getReferenceRows(int row) const;
    int calculateNextNumRows() const;
    int calculatePrevNumRows() const;
    
    int numRows = 3;           // Always odd: 3, 5, 9, 17, etc.
    int selectedRow = 1;       // Currently selected row (for UI)
};
