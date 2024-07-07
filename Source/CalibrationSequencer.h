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
    enum class SequenceType
    {
        INIT_AMPLITUDE_WINDOWS,
        AMPLITUDE,
        PAN,
        PHASE
    };
    
    struct Sequence
    {
        Sequence (SequenceType type, float precision) : type (type), precision (precision) {}
        
        SequenceType type;
        float precision;
    };
    
    static constexpr float REFERENCE_FREQ = 1000.0;
    static constexpr float REFERENCE_GAIN_DB = 0.0;
    static constexpr float PHASE_LOW_HZ = 30;
    static constexpr float PHASE_HIGH_HZ = 1400; // range of freqs to poll for phase
    
    const CalibratedSetPointManager& calibratedSetPointManager;
    
    bool hasSequenceCompleted (Sequence sequence);
    Question executeSequence(Sequence sequence);
    
    Question phaseQuestion (const float frequency, const float gain, const PhaseCalibratedSetPoint phase) const;
    
    bool referenceNoteHasBeenCalibrated() const;
    bool amplitudesHaveBeenWindowed() const;
    bool amplitudesHaveUpperBounds() const;
    bool phasesHaveBeenCalibratedWithPrecision (float precision) const;
    bool pansHaveBeenCalibratedWithPrecision (float precision) const;
    bool amplitudesHaveBeenCalibratedWithPrecision (float precision) const;
    
    CalibratedSetPoint getRandomLowPrecisionSetPoint (const std::vector<CalibratedSetPoint>& calibratedSetPoints) const; // gets random set point of all those w/ least precision
    
    const Note referenceNote() const;
    const CalibratedSetPoint& getReferencePan() const;
    const CalibratedSetPoint& getReferencePhase() const;
};
