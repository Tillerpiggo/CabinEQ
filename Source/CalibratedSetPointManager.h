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
#include "PhaseCalibratedSetPoint.h"
#include "Question.h"

class CalibratedSetPointManager
{
public:
    CalibratedSetPointManager();
    
    const Curve& getCurve() const;
    void updateCurve (Curve& curve) const;
    
    void calibrateWith (Question question, CalibrationChoice choice);
    void calibrateSetPointWith (CalibratedSetPoint calibratedSetPoint, CalibrationChoice choice, float value, float interpolatedValue);
    
    void changeReferencePanTo (float newReferencePan);
    
    const float amplitudeAt (const float frequency) const;
    const float panAt (const float frequency) const;
    const float phaseAt (const float frequency) const;
    
    const std::vector<float>& getFrequencies() const;
    const std::vector<CalibratedSetPoint>& getAmplitudes() const { return amplitudes; }
    const std::vector<CalibratedSetPoint>& getAmplitudeCalibratedSetPoints() const;
    
    void setAmplitudeAt (int index, float value);
    
    bool amplitudesHaveBeenWindowed() const;
    bool amplitudesHaveUpperBounds() const;
    bool pansHaveBeenWindowed() const;
    bool amplitudesHaveBeenCalibratedWithPrecision (float precision) const;
    bool pansHaveBeenCalibratedWithPrecision (float precision) const;
    bool phasesHaveBeenCalibratedWithPrecision (float precision) const;
    bool phasesHaveBeenCalibratedWithPrecisionInHzRange (float precision, float lowerHz, float upperHz) const;
    
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionAmplitude() const;
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionPan() const;
    std::pair<float, CalibratedSetPoint> getFirstAmplitudeWithPrecisionLessThan (float precision) const;
    float interpolateAmplitudeAt (float frequency) const;
    float interpolatePanAt (float frequency) const;
    std::pair<float, CalibratedSetPoint> getFirstPanWithPrecisionLessThan (float precision) const;
    std::pair<float, PhaseCalibratedSetPoint> getRandomLowestPrecisionPhaseInRange (float lowerHz, float upperHz) const;
    
    const CalibratedSetPoint& getReferencePanCalibratedSetPoint() const;
    const CalibratedSetPoint& getReferencePhaseCalibratedSetPoint() const;
    
private:
    static constexpr float REFERENCE_PAN_WINDOW = 3.0;
    static constexpr float REFERENCE_PHASE_WINDOW = 3.14;
    
    bool calibratedSetPointsHaveBeenWindowed (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const;
    bool calibratedSetPointsHaveBeenCalibratedWithPrecision (const std::vector<CalibratedSetPoint>& calibratedSetPoints,
                                                             float precision) const;
    bool calibratedSetPointsHaveBeenCalibratedWithPrecisionInHzRange (const std::vector<CalibratedSetPoint>& calibratedSetPoints,
                                                                      float precision,
                                                                      float lowerHz,
                                                                      float upperHz) const;
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionCalibratedSetPoint (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const; // returns (frequency, calibratedSetPoint)
    std::pair<float, CalibratedSetPoint> getRandomLowestPrecisionCalibratedSetPointInRange (const std::vector<CalibratedSetPoint>& calibratedSetPoints, float lowerHz, float upperHz) const;
    
    int indexForFrequency (float frequency) const;
    
    std::vector<float> frequencies;
    std::vector<CalibratedSetPoint> amplitudes;
    std::vector<CalibratedSetPoint> pans;
    std::vector<PhaseCalibratedSetPoint> phases;
    
    Curve curve;
    CalibratedSetPoint referencePan;
    CalibratedSetPoint referencePhase;
    
};

