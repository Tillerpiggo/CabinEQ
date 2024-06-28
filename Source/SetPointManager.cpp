/*
  ==============================================================================

    SetPointManager.cpp
    Created: 12 Jun 2024 10:41:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SetPointManager.h"


#include <cmath>

//std::vector<float> SetPointManager::initializeSetPointFreqs(int numPoints)
//{
//    // Initialize constants
//    float minFreq = 20.f;
//    float maxFreq = 15000.f;
//
//    // Create the set points
//    std::vector<float> setPoints;
//
//    // Exponential spacing
//    for (int i = 0; i < numPoints; i++)
//    {
//        float t = static_cast<float>(i) / (numPoints - 1); // Normalized position between 0 and 1
//        float currFreq = minFreq * pow(maxFreq / minFreq, t);
//        setPoints.push_back(currFreq);
//    }
//
//    return setPoints;
//}

std::vector<float> SetPointManager::initializeSetPointFreqs (int numPoints)
{
    std::vector<float> initialSetPointFreqs { 10, 15, 20, 30, 40, 60, 80, 100, 120, 140, 160, 180, 200, 250, 300, 350, 400, 450, 500,
    600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1700, 1850, 2000, 2250, 2500, 3000, 3500, 4000, 4500, 5000,
    6000, 6250, 6500, 6750, 7000, 7250, 7500, 7750, 8000, 8250, 8500, 8750, 9000, 9250, 9500, 9750, 10000, 10500, 11000, 11500, 12000, 12500, 13000, 13500, 14000, 14500, 15000,
    15500, 16000, 16500, 17000, 17500, 18000, 18500};
    
    return initialSetPointFreqs;
}

//std::vector<float> SetPointManager::initializeSetPointFreqs (int numPoints)
//{
//    // Initialize constants
//    float minFreq = 10.f;
//    float midFreq = 3000.f;
//    float upperMidFreq = 8000.f; // Upper limit for exponential segment
//    float maxFreq = 18000.f;
//
//    // Determine the number of points for the fixed spacing segment (8000 Hz to 15000 Hz)
//    float fixedSpacing = 150.f;
//    int numPointsFixed = static_cast<int>((maxFreq - upperMidFreq) / fixedSpacing) + 1;
//
//    // Adjust the number of points for the other segments
//    int remainingPoints = numPoints - numPointsFixed;
//    float ratio = log10(midFreq / minFreq) / (log10(midFreq / minFreq) + log10(upperMidFreq / midFreq));
//    int numPointsLogarithmic = static_cast<int>(remainingPoints * ratio);
//    int numPointsExponential = remainingPoints - numPointsLogarithmic;
//
//    // Create the set points
//    std::vector<float> setPoints;
//
//    // Logarithmic spacing for the first segment
//    for (int i = 0; i < numPointsLogarithmic; i++)
//    {
//        float t = static_cast<float>(i) / (numPointsLogarithmic - 1); // Normalized position between 0 and 1
//        float currFreq = minFreq * pow(midFreq / minFreq, t);
//        setPoints.push_back(currFreq);
//    }
//
//    // Exponential spacing for the second segment
//    for (int i = 0; i < numPointsExponential; i++)
//    {
//        float t = static_cast<float>(i) / (numPointsExponential - 1); // Normalized position between 0 and 1
//        float currFreq = midFreq * pow(upperMidFreq / midFreq, t);
//        setPoints.push_back(currFreq);
//    }
//
//    // Fixed spacing for the third segment (8000 Hz to 15000 Hz)
//    for (int i = 0; i < numPointsFixed; i++)
//    {
//        float currFreq = upperMidFreq + i * fixedSpacing;
//        setPoints.push_back(currFreq);
//    }
//
//    return setPoints;
//}

void SetPointManager::updateValueAtIdx (int idx, float gain)
{
    if (idx < 0 || idx >= NUM_SET_POINTS)
    {
        std::cerr << "ERROR: Index is out of range in SetPointManager::updateGainAtIdx" << std::endl;
        return;
    }
    
    setPointGains[idx] = gain;
    
    //std::cout << "gain at idx " << idx << ": " << gain << std::endl;
}
