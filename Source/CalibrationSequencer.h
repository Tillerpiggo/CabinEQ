/*
  ==============================================================================

    CalibrationSequencer.h
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibratedSetPointManager.h"
#include "Note.h"
#include "Question.h"

class CalibrationSequencer
{
public:
    CalibrationSequencer (const CalibratedSetPointManager& calibratedSetPointManager);
    Question getNextQuestion();
    
private:
    static constexpr float REFERENCE_FREQ = 1000.0;
    static constexpr float REFERENCE_GAIN_DB = 0.0;
    static constexpr float REFERENCE_PAN_WINDOW = 3.0;
    static constexpr float REFERENCE_PHASE_WINDOW = 3.14;
    
    static constexpr float AMPLITUDE_PRECISION = 0.05;
    static constexpr float PAN_PRECISION = 0.01;
    static constexpr float PHASE_PRECISION = 0.01;
    
    const CalibratedSetPointManager& calibratedSetPointManager;
    
    bool referenceNoteHasBeenCalibrated() const;
    bool amplitudesHaveBeenWindowed() const;
    bool phasesHaveBeenCalibratedPrecisely() const;
    bool pansHaveBeenCalibratedPrecisely() const;
    bool amplitudesHaveBeenCalibratedPrecisely() const;
    
    CalibratedSetPoint getRandomLowPrecisionSetPoint (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const; // gets random set point of all those w/ least precision
    
    Note referenceNote();
    CalibratedSetPoint referencePan;
    CalibratedSetPoint referencePhase;
    
};
