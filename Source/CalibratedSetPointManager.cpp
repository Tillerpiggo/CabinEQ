/*
  ==============================================================================

    CalibratedSetPointManager.cpp
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibratedSetPointManager.h"
#include <random>

CalibratedSetPointManager::CalibratedSetPointManager() : curve (frequencies, amplitudes, phases, pans),
                                                         referencePan (REFERENCE_PAN_WINDOW),
                                                         referencePhase (REFERENCE_PHASE_WINDOW)
{
    // TODO: Construct the first batch of set points...
    //frequencies = { 300, 2000, 5000 };//{ 20, 100, 300, 600, 2000, 3000, 6000, 10000, 12500, 15000 };
    frequencies = { 20, 40, 80, 160, 240, 320, 640, 1280, 1800, 2560, 3500, 4000, 5120, 6000, 7000, 8000, 9000, 10240, 11000, 12000, 13000, 14000, 15000 };
    
    
//    frequencies = { 20, 40, 60, 80, 100, 200, 300, 400, 500, 650, 800, 1280, 1500, 1800, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 10500, 11000, 11500,
//        12000, 12500, 13000, 13500, 14000, 14500, 15000 };
    
//    frequencies = { 160 };
    //frequencies = { 640 };
    
    for (int i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back (CalibratedSetPoint (12.0f));
        pans.push_back (CalibratedSetPoint (6.0f));
        phases.push_back (PhaseCalibratedSetPoint());
    }
    
    referencePan.setValue (2.0);
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
    
    std::cout << "Calibrating freq " << freq << std::endl;
    
    if (idx < 0)
    {
        return;
    }
    
    switch (question.getType())
    {
        case QuestionType::Level:
            amplitudes.at (idx).calibrateWith (choice, question.getNote2().gain);
            break;
        case QuestionType::Pan:
            pans.at (idx).calibrateWith (choice, question.getNote2().pan);
            break;
        case QuestionType::Phase:
            phases.at (idx).calibrateWith (choice);
            break;
        case QuestionType::ReferencePan:
            referencePan.calibrateWith (choice, question.getNote2().pan); // NOTE: This code is BROKEN!!!!
            break;
        case QuestionType::ReferencePhase:
            //referencePhase.calibrateWith (choice);  // NOTE: This code is BROKEN TOO!!!!
            break;
    }
}

void CalibratedSetPointManager::changeReferencePanTo (float newReferencePan)
{
    referencePan.setValue (newReferencePan);
}

const float CalibratedSetPointManager::amplitudeAt (const float frequency) const
{
    int freqIdx = indexForFrequency (frequency);
    CalibratedSetPoint amplitudeCalibratedSetPoint = amplitudes.at (indexForFrequency (frequency));
    return amplitudeCalibratedSetPoint.estimatedValue();
    if (amplitudeCalibratedSetPoint.getIsCompletelyUncalibrated())
    {
        // Return average of surrounding set points
        if (freqIdx == 0)
        {
            return amplitudes.at (freqIdx + 1).estimatedValue();
        }
        
        if (freqIdx == frequencies.size() - 1)
        {
            return amplitudes.at (freqIdx - 1).estimatedValue();
        }
        
        return (amplitudes.at (freqIdx - 1).estimatedValue() + amplitudes.at (freqIdx + 1).estimatedValue()) / 2.0f;
    }
    else
    {
        return amplitudeCalibratedSetPoint.estimatedValue();
    }
}

const float CalibratedSetPointManager::panAt (const float frequency) const
{
    return pans.at (indexForFrequency (frequency)).estimatedValue();
}

const float CalibratedSetPointManager::phaseAt (const float frequency) const
{
    return phases.at (indexForFrequency (frequency)).estimatedValue();
}

const std::vector<float>& CalibratedSetPointManager::getFrequencies() const
{
    return frequencies;
}

const std::vector<CalibratedSetPoint>& CalibratedSetPointManager::getAmplitudeCalibratedSetPoints() const
{
    return amplitudes;
}

void CalibratedSetPointManager::setAmplitudeAt (int index, float value)
{
    amplitudes.at (index).setValue (value);
}

const CalibratedSetPoint& CalibratedSetPointManager::getReferencePanCalibratedSetPoint() const
{
    return referencePan;
}

const CalibratedSetPoint& CalibratedSetPointManager::getReferencePhaseCalibratedSetPoint() const
{
    return referencePhase;
}

void CalibratedSetPointManager::setFletcherMunsonCompensation (float calibrationDB, float calibrationFactor, float fmDB)
{
    curve.setFletcherMunsonCompensation (calibrationDB, calibrationFactor, fmDB);
}

bool CalibratedSetPointManager::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointsHaveBeenWindowed (amplitudes);
}

bool CalibratedSetPointManager::amplitudesHaveUpperBounds() const
{
    for (auto& amplitude : amplitudes)
    {
        if (amplitude.getUpperBound() == std::numeric_limits<float>::infinity())
        {
            return false;
        }
    }
    
    return true;
}

bool CalibratedSetPointManager::pansHaveBeenWindowed() const
{
    return calibratedSetPointsHaveBeenWindowed (pans);
}

bool CalibratedSetPointManager::amplitudesHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointsHaveBeenCalibratedWithPrecision (amplitudes, precision);
}

bool CalibratedSetPointManager::pansHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointsHaveBeenCalibratedWithPrecision (pans, precision);
}

bool CalibratedSetPointManager::phasesHaveBeenCalibratedWithPrecisionInHzRange(
                                                                  float precision,
                                                                  float lowerHz,
                                                                  float upperHz) const
{
    for (int i = 0; i < frequencies.size(); ++i)
    {
        PhaseCalibratedSetPoint phaseCalibratedSetPoint = phases.at (i);
        float frequency = frequencies.at (i);
        
        bool isImprecise = phaseCalibratedSetPoint.precision() > precision;
        bool inRange = frequency >= lowerHz && frequency <= upperHz;
        if (isImprecise && inRange)
        {
            return false;
        }
    }
    
    return true;
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionAmplitude() const
{
    return getRandomLowestPrecisionCalibratedSetPoint (amplitudes);
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionPan() const
{
    return getRandomLowestPrecisionCalibratedSetPoint (pans);
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getFirstAmplitudeWithPrecisionLessThan (float precision) const
{
    for (int i = 0; i < frequencies.size(); ++i)
    {
        if (amplitudes.at (i).precision() > precision)
        {
            return { frequencies.at (i), amplitudes.at (i) };
        }
    }
    
    return { frequencies.at (0), amplitudes.at (0) };
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getFirstPanWithPrecisionLessThan (float precision) const
{
    for (int i = 0; i < frequencies.size(); ++i)
    {
        if (pans.at (i).precision() > precision)
        {
            return { frequencies.at (i), pans.at (i) };
        }
    }
    
    return { frequencies.at (0), pans.at (0) };
}

float CalibratedSetPointManager::interpolateAmplitudeAt (float frequency) const
{
    int idx = indexForFrequency (frequency);

    if (idx == 0)
    {
        return amplitudes.at (idx).estimatedValue();
    }

    return amplitudes.at (idx - 1).estimatedValue();
}

float CalibratedSetPointManager::interpolatePanAt (float frequency) const
{
    int idx = indexForFrequency (frequency);

    if (idx == 0)
    {
        return pans.at (idx).estimatedValue();
    }

    return pans.at (idx - 1).estimatedValue();
}

std::pair<float, PhaseCalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionPhaseInRange (float lowerHz, float upperHz) const
{
    if (phases.empty()) {
        throw std::runtime_error("Called getRandomLowestPrecisionCalibratedSetPointInRange on empty vector");
    }

    // Filter set points based on the frequency range
    std::vector<size_t> validIndices;
    for (size_t i = 0; i < phases.size(); ++i) {
        float frequency = frequencies[i];
        if (frequency >= lowerHz && frequency <= upperHz) {
            validIndices.push_back(i);
        }
    }

    if (validIndices.empty()) {
        throw std::runtime_error("No calibrated set points in the specified frequency range");
    }

    // Find the minimum precision among the valid set points
    float minPrecision = 0.0f;
    for (size_t index : validIndices) {
        float precision = phases[index].precision();
        if (precision > minPrecision) {
            minPrecision = precision;
        }
    }

    // Get indices of all valid calibrated set points with the minimum precision
    std::vector<size_t> minPrecisionIndices;
    for (size_t index : validIndices) {
        if (phases[index].precision() == minPrecision) {
            minPrecisionIndices.push_back(index);
        }
    }

    // Randomly pick one from the min precision indices
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(minPrecisionIndices.size()) - 1);
    size_t chosenIndex = minPrecisionIndices[dis(gen)];

    // Get corresponding frequency and calibrated set point
    const PhaseCalibratedSetPoint& chosenSetPoint = phases[chosenIndex];
    float chosenFrequency = frequencies[chosenIndex];
    
    return { chosenFrequency, chosenSetPoint };
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
    return calibratedSetPointsHaveBeenCalibratedWithPrecisionInHzRange (calibratedSetPoints,
                                                                        precision,
                                                                        0,
                                                                        30000); // accept all frequencies
}

bool CalibratedSetPointManager::calibratedSetPointsHaveBeenCalibratedWithPrecisionInHzRange (const std::vector<CalibratedSetPoint>& calibratedSetPoints,
                                                                  float precision,
                                                                  float lowerHz,
                                                                  float upperHz) const
{
    for (int i = 0; i < frequencies.size(); ++i)
    {
        CalibratedSetPoint calibratedSetPoint = calibratedSetPoints.at (i);
        float frequency = frequencies.at (i);
        
        bool isImprecise = ! calibratedSetPoint.hasEstablishedWindow() ||
                           calibratedSetPoint.precision() > precision;
        
        std::cout << "precision at freq " << frequencies.at (i) << ": " << calibratedSetPoint.precision();
        bool inRange = frequency >= lowerHz && frequency <= upperHz;
        if (isImprecise && inRange)
        {
            return false;
        }
    }
    
    return true;
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionCalibratedSetPoint(
    const std::vector<CalibratedSetPoint>& calibratedSetPoints) const 
{
    
    return getRandomLowestPrecisionCalibratedSetPointInRange (calibratedSetPoints, 0, 30000);
}

std::pair<float, CalibratedSetPoint> CalibratedSetPointManager::getRandomLowestPrecisionCalibratedSetPointInRange(
    const std::vector<CalibratedSetPoint>& calibratedSetPoints, float lowerHz, float upperHz) const 
{
    
    if (calibratedSetPoints.empty()) {
        throw std::runtime_error("Called getRandomLowestPrecisionCalibratedSetPointInRange on empty vector");
    }

    // Filter set points based on the frequency range
    std::vector<size_t> validIndices;
    for (size_t i = 0; i < calibratedSetPoints.size(); ++i) {
        float frequency = frequencies[i];
        if (frequency >= lowerHz && frequency <= upperHz) {
            validIndices.push_back(i);
        }
    }

    if (validIndices.empty()) {
        throw std::runtime_error("No calibrated set points in the specified frequency range");
    }

    // Find the minimum precision among the valid set points
    float minPrecision = 0.0f;
    for (size_t index : validIndices) {
        float precision = calibratedSetPoints[index].precision();
        if (precision > minPrecision) {
            minPrecision = precision;
        }
    }
    
    // Return the first one
    for (size_t index : validIndices)
    {
        if (calibratedSetPoints[index].precision() == minPrecision)
        {
            const CalibratedSetPoint& chosenSetPoint = calibratedSetPoints[index];
            float chosenFrequency = frequencies[index];
            return { chosenFrequency, chosenSetPoint };
        }
    }

//    // Get indices of all valid calibrated set points with the minimum precision
//    std::vector<size_t> minPrecisionIndices;
//    for (size_t index : validIndices) {
//        if (calibratedSetPoints[index].precision() == minPrecision) {
//            minPrecisionIndices.push_back(index);
//        }
//    }
//
//    // Randomly pick one from the min precision indices
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> dis(0, static_cast<int>(minPrecisionIndices.size()) - 1);
//    size_t chosenIndex = minPrecisionIndices[dis(gen)];
//
//    // Get corresponding frequency and calibrated set point
//    const CalibratedSetPoint& chosenSetPoint = calibratedSetPoints[chosenIndex];
//    float chosenFrequency = frequencies[chosenIndex];
    
    //return { chosenFrequency, chosenSetPoint };
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
