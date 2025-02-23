/*
  ==============================================================================

    ElevationCalibrationManager.cpp
    Created: 23 Feb 2025 10:25:30am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ElevationCalibrationManager.h"

ElevationCalibrationManager::ElevationCalibrationManager()
{
}

void ElevationCalibrationManager::incrementRows()
{
    if (canIncrementRows())
    {
        calibration.incrementRows();
    }
}

void ElevationCalibrationManager::decrementRows()
{
    if (canDecrementRows())
    {
        calibration.decrementRows();
    }
}

bool ElevationCalibrationManager::canIncrementRows() const
{
    return calibration.canIncrementRows();
}

bool ElevationCalibrationManager::canDecrementRows() const
{
    return calibration.canDecrementRows();
}

void ElevationCalibrationManager::setSelectedRow(int row)
{
    calibration.setSelectedRow(row);
}

int ElevationCalibrationManager::getNumRows()
{
    return calibration.getNumRows();
}

int ElevationCalibrationManager::getSelectedRow()
{
    return calibration.getSelectedRow();
}

bool ElevationCalibrationManager::isPlaying()
{
    return playing;
}

int ElevationCalibrationManager::getPlayingRowAtTime(double normalizedTime) const
{
    return calibration.getPlayingRowAtTime(normalizedTime);
}
