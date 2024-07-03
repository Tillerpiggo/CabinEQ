/*
  ==============================================================================

    CalibratedSetPointManager.h
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Curve.h"
#include "CalibratedSetPoint.h"
#include "Question.h"

class CalibratedSetPointManager
{
public:
    CalibratedSetPointManager();
    
    const Curve& getCurve() const;
    
    void calibrateWith (Question question, CalibrationChoice choice);
    
    const float amplitudeAt (const float frequency) const;
    const float panAt (const float frequency) const;
    const float phaseAt (const float frequency) const;
    
    bool amplitudesHaveBeenWindowed() const;
    bool pansHaveBeenWindowed() const;
    bool phasesHaveBeenWindowed() const;
    bool amplitudesHaveBeenCalibratedWithPrecision (float precision) const;
    bool pansHaveBeenCalibratedWithPrecision (float precision) const;
    bool phasesHaveBeenCalibratedWithPrecision (float precision) const;
    
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionAmplitude() const;
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionPan() const;
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionPhase() const;
    
private:
    bool calibratedSetPointsHaveBeenWindowed (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const;
    bool calibratedSetPointsHaveBeenCalibratedWithPrecision (const std::vector<CalibratedSetPoint>& calibratedSetPoints,
                                                             float precision) const;
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionCalibratedSetPoint (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const; // returns (frequency, calibratedSetPoint)
    
    int indexForFrequency (float frequency) const;
    
    std::vector<float> frequencies;
    std::vector<CalibratedSetPoint> amplitudes;
    std::vector<CalibratedSetPoint> pans;
    std::vector<CalibratedSetPoint> phases;
    
    Curve curve;
};

