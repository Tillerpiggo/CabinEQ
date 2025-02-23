/*
  ==============================================================================

    ElevationCalibrationManager.h
    Created: 23 Feb 2025 10:25:30am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "ElevationCalibration.h"
#include "Listeners.h"

class ElevationCalibrationManager
{
public:
    ElevationCalibrationManager();
    
    // Row management
    void incrementRows();
    void decrementRows();
    bool canIncrementRows() const;
    bool canDecrementRows() const;
    
    // Row selection
    void setSelectedRow(int row);
    
    // ElevationCalibrationDataSource
    int getNumRows();
    int getSelectedRow();
    int getPlayingRowAtTime(float normalizedTime);
    ElevationCalibration getCurrElevationCalibration();
    
    int getPlayingRowAtTime(double normalizedTime) const;
    
private:
    ElevationCalibration calibration;
};
