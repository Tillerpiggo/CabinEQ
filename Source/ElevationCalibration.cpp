/*
  ==============================================================================

    ElevationCalibration.cpp
    Created: 23 Feb 2025 10:25:13am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ElevationCalibration.h"
#include <cmath>

ElevationCalibration::ElevationCalibration()
{
}

bool ElevationCalibration::canIncrementRows() const
{
    return calculateNextNumRows() <= 17; // Arbitrary max
}

bool ElevationCalibration::canDecrementRows() const
{
    return numRows > 3;
}

void ElevationCalibration::incrementRows()
{
    if (canIncrementRows())
        numRows = calculateNextNumRows();
}

void ElevationCalibration::decrementRows()
{
    if (canDecrementRows())
        numRows = calculatePrevNumRows();
}

void ElevationCalibration::setSelectedRow(int row)
{
    selectedRow = std::clamp(row, 0, numRows - 1);
}

int ElevationCalibration::getPlayingRowAtTime(double normalizedTime) const
{
    std::vector<int> rowsToPlay;
    if (selectedRow == 0 || selectedRow == numRows-1 || selectedRow == numRows/2)
    {
        rowsToPlay = getReferenceRows(selectedRow);
    }
    else
    {
        rowsToPlay = getAdjacentRows(selectedRow);
    }
    
    int numPlayingRows = static_cast<int>(rowsToPlay.size());
    int currentRowIndex = static_cast<int>(normalizedTime * numPlayingRows) % numPlayingRows;
    
    return rowsToPlay[currentRowIndex];
}

int ElevationCalibration::getPlayingColAtTime(double normalizedTime) const
{
    float normalizedRowTime = std::fmod(normalizedTime, 1.0f / static_cast<float>(numCols));
    normalizedRowTime *= static_cast<float>(numCols);
    return static_cast<int>(std::clamp(normalizedRowTime * numCols, 0.0f, static_cast<float>(numCols-1)));
}

std::vector<int> ElevationCalibration::getAdjacentRows(int row) const
{
    return { row - 1, row, row + 1 };
}

std::vector<int> ElevationCalibration::getReferenceRows(int row) const
{
    return { 0, numRows/2, numRows-1 };
}

int ElevationCalibration::calculateNextNumRows() const
{
    return numRows * 2 - 1;
}

int ElevationCalibration::calculatePrevNumRows() const
{
    return (numRows + 1) / 2;
}
