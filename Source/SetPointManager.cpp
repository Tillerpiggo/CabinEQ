/*
  ==============================================================================

    SetPointManager.cpp
    Created: 12 Jun 2024 10:41:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SetPointManager.h"


#include <cmath>

std::vector<float> SetPointManager::initializeSetPointFreqs (int numPoints)
{
    // Initialize constants
    float minFreq = 10.f;
    float midFreq = 3000.f;
    float upperMidFreq = 8000.f; // Upper limit for exponential segment
    float maxFreq = 18000.f;

    // Determine the number of points for the fixed spacing segment (8000 Hz to 15000 Hz)
    float fixedSpacing = 150.f;
    int numPointsFixed = static_cast<int>((maxFreq - upperMidFreq) / fixedSpacing) + 1;

    // Adjust the number of points for the other segments
    int remainingPoints = numPoints - numPointsFixed;
    float ratio = log10(midFreq / minFreq) / (log10(midFreq / minFreq) + log10(upperMidFreq / midFreq));
    int numPointsLogarithmic = static_cast<int>(remainingPoints * ratio);
    int numPointsExponential = remainingPoints - numPointsLogarithmic;

    // Create the set points
    std::vector<float> setPoints;

    // Logarithmic spacing for the first segment
    for (int i = 0; i < numPointsLogarithmic; i++)
    {
        float t = static_cast<float>(i) / (numPointsLogarithmic - 1); // Normalized position between 0 and 1
        float currFreq = minFreq * pow(midFreq / minFreq, t);
        setPoints.push_back(currFreq);
    }

    // Exponential spacing for the second segment
    for (int i = 0; i < numPointsExponential; i++)
    {
        float t = static_cast<float>(i) / (numPointsExponential - 1); // Normalized position between 0 and 1
        float currFreq = midFreq * pow(upperMidFreq / midFreq, t);
        setPoints.push_back(currFreq);
    }

    // Fixed spacing for the third segment (8000 Hz to 15000 Hz)
    for (int i = 0; i < numPointsFixed; i++)
    {
        float currFreq = upperMidFreq + i * fixedSpacing;
        setPoints.push_back(currFreq);
    }

    return setPoints;
}

void SetPointManager::updateGainAtIdx (int idx, float gain)
{
    if (idx < 0 || idx >= NUM_SET_POINTS)
    {
        std::cerr << "ERROR: Index is out of range in SetPointManager::updateGainAtIdx" << std::endl;
        return;
    }
    
    setPointGains[idx] = gain;
}
