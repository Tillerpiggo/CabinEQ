/*
  ==============================================================================

    CalibratedSetPointManager.cpp
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPointManager.h"
#include <random>

CalibratedSetPointManager::CalibratedSetPointManager() : curve(frequencies, amplitudes, pans, phases)
{
    // TODO: Construct the first batch of set points...
    frequencies = { 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240 };
    
    for (int i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back (CalibratedSetPoint (12.0f));
        pans.push_back (CalibratedSetPoint (3.0f));
        phases.push_back (CalibratedSetPoint (-3.14f, 3.14f));
    }
}

const Curve& CalibratedSetPointManager::getCurve() const
{
    return curve;
}

void CalibratedSetPointManager::calibrateWith (Question question, CalibrationChoice choice)
{
    // Get the idx for the question and update the corresponding set point
    float freq = question.controlledFrequency();
    int idx = indexForFrequency (freq);
    
    switch (question.getType())
    {
        case QuestionType::Level:
            amplitudes.at (idx).calibrateWith (choice);
            break;
        case QuestionType::Pan:
            pans.at (idx).calibrateWith (choice);
            break;
        case QuestionType::Phase:
            phases.at (idx).calibrateWith (choice);
            break;
        case QuestionType::HigherThan:
            float gain = question.getNote2().gain;
            amplitudes.at (idx).setWindow (gain - 6.0f, gain);
            break;
    }
}

const float CalibratedSetPointManager::amplitudeAt (const float frequency) const
{
    return amplitudes.at (indexForFrequency (frequency)).estimatedValue();
}

const float CalibratedSetPointManager::panAt (const float frequency) const
{
    return pans.at (indexForFrequency (frequency)).estimatedValue();
}

const float CalibratedSetPointManager::phaseAt (const float frequency) const
{
    return phases.at (indexForFrequency (frequency)).estimatedValue();
}

bool CalibratedSetPointManager::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointsHaveBeenWindowed (amplitudes);
}

bool CalibratedSetPointManager::pansHaveBeenWindowed() const
{
    return calibratedSetPointsHaveBeenWindowed (pans);
}

bool CalibratedSetPointManager::phasesHaveBeenWindowed() const
{
    return calibratedSetPointsHaveBeenWindowed (phases);
}

bool CalibratedSetPointManager::amplitudesHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointsHaveBeenCalibratedWithPrecision (amplitudes, precision);
}

bool CalibratedSetPointManager::pansHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointsHaveBeenCalibratedWithPrecision (pans, precision);
}

bool CalibratedSetPointManager::phasesHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointsHaveBeenCalibratedWithPrecision (phases, precision);
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionAmplitude() const
{
    return getRandomLowestPrecisionCalibratedSetPoint (amplitudes);
}
std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionPan() const
{
    return getRandomLowestPrecisionCalibratedSetPoint (pans);
}
std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionPhase() const
{
    return getRandomLowestPrecisionCalibratedSetPoint (phases);
}

bool CalibratedSetPointManager::calibratedSetPointsHaveBeenWindowed (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const
{
    for (const auto& calibratedSetPoint : calibratedSetPoints) 
    {
        if (! calibratedSetPoint.hasEstablishedWindow())
        {
            return false;
        }
    }
    return true;
}

bool CalibratedSetPointManager::calibratedSetPointsHaveBeenCalibratedWithPrecision (const std::vector<CalibratedSetPoint>& calibratedSetPoints, float precision) const
{
    for (const auto& calibratedSetPoint : calibratedSetPoints)
    {
        if (! calibratedSetPoint.hasEstablishedWindow() ||
            calibratedSetPoint.precision() > precision)
        {
            return false;
        }
    }
    return true;
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionCalibratedSetPoint(const std::vector<CalibratedSetPoint>& calibratedSetPoints) const {
    if (calibratedSetPoints.empty()) {
        throw std::runtime_error("Called getRandomLowestPrecisionCalibratedSetPoint on empty vector");
    }

    // Find the minimum precision
    float minPrecision = std::numeric_limits<float>::max();
    for (const auto& setPoint : calibratedSetPoints) {
        float precision = setPoint.precision();
        if (precision < minPrecision) {
            minPrecision = precision;
        }
    }

    // Get indices of all calibrated set points w/ minimum precision
    std::vector<size_t> minPrecisionIndices;
    for (size_t i = 0; i < calibratedSetPoints.size(); ++i) {
        if (calibratedSetPoints[i].precision() == minPrecision) {
            minPrecisionIndices.push_back(i);
        }
    }

    // Randomly pick one
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(minPrecisionIndices.size()) - 1);
    size_t chosenIndex = minPrecisionIndices[dis(gen)];

    // Get corresponding frequency/calibrated set point
    const CalibratedSetPoint& chosenSetPoint = calibratedSetPoints[chosenIndex];
    float chosenFrequency = frequencies[chosenIndex];

    return { chosenFrequency, chosenSetPoint };
}

int CalibratedSetPointManager::indexForFrequency (float frequency) const
{
    for (int i = 0; i < frequencies.size(); ++i)
    {
        if (frequency == frequencies.at (i))
        {
            return i;
        }
    }
    
    return -1;
}
