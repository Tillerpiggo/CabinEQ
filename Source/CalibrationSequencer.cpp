/*
  ==============================================================================

    CalibrationSequencer.cpp
    Created: 1 Jul 2024 3:34:30pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationSequencer.h"
#include <random>

CalibrationSequencer::CalibrationSequencer (const CalibratedSetPointManager& calibratedSetPointManager)
: calibratedSetPointManager (calibratedSetPointManager)
{
    
}

Question CalibrationSequencer::getNextQuestion()
{
    // TODO: Figure out what state we need to track to implement a basic calibration sequence
    // For now, just repeatedly ask for panning, then phase, then level of the 11 notes
    if (! referenceNoteHasBeenCalibrated())
    {
        CalibratedSetPoint referencePan = getReferencePan();
        CalibratedSetPoint referencePhase = getReferencePhase();
        
        // Calibrate pan or phase (choose randomly)
        if (referencePhase.precision() > 0.05)
        {
            Note note1 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        0.0f,
                        referencePhase.estimatedValue() - referencePhase.getWindowSize());
            
            Note note2 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        0.0f,
                        referencePhase.estimatedValue() + referencePhase.getWindowSize());
            
            
            
            return Question (QuestionType::ReferencePhase, note1, note2);
        }
        else
        {
            Note note1 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() - referencePan.getWindowSize(),
                        0.0f);
            
            Note note2 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() + referencePan.getWindowSize(),
                        0.0f);
            
            return Question (QuestionType::ReferencePan, note1, note2);
        }
        
       
    }
    else if (! amplitudesHaveBeenWindowed())
    {
        std::cout << "leveling initial amplitude" << std::endl;
        
        // Get random next amplitude to test
        auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
        
        Note note1 = referenceNote();
        Note note2 (freq, amplitudeCalibratedSetPoint.estimatedValue(), 0.0f, 0.0f);
        
        return Question (QuestionType::HigherThan, note1, note2);
    }
    else if (! phasesHaveBeenCalibratedPrecisely())
    {
        std::cout << "calibrating phase" << std::endl;
        // Get random next phase to calibrate
        
        // Hacky but whatever, get in range (40, 2000)
        auto [freq, phaseCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPhaseInRange (PHASE_LOW_HZ, PHASE_HIGH_HZ);
        Question phaseQuestionVar = phaseQuestion (freq, calibratedSetPointManager.amplitudeAt (freq), phaseCalibratedSetPoint);
//        std::cout << "Phase question >" << std::endl;
//        std::cout << "Note1: freq = " << phaseQuestionVar.getNote1().frequency
//        << ", phase = " << phaseQuestionVar.getNote1().phase << std::endl;
//        std::cout << "Note2: freq = " << phaseQuestionVar.getNote2().frequency
//        << ", phase = " << phaseQuestionVar.getNote2().phase << std::endl;
        
        return phaseQuestionVar;
    }
    else if (! pansHaveBeenCalibratedPrecisely())
    {
        std::cout << "calibrating pan" << std::endl;
        // Get random next pan to calibrate
        auto [freq, panCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPan();
        
        Note note1 = referenceNote();
        Note note2 (freq, calibratedSetPointManager.amplitudeAt (freq), panCalibratedSetPoint.estimatedValue(), 0.0f);
        
        return Question (QuestionType::Pan, note1, note2);
    }
    else if (! amplitudesHaveBeenCalibratedPrecisely())
    {
        std::cout << "calibrating level (final)" << std::endl;
        // Get random next level to calibrate
        auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
        
        Note note1 = referenceNote();
        Note note2 (freq, amplitudeCalibratedSetPoint.estimatedValue(), 0.0f, 0.0f);
        
        return Question (QuestionType::Level, note1, note2);
    }
    else
    {
        std::cout << "done; going to default question" << std::endl;
        return Question::defaultQuestion();
    }
}

Question CalibrationSequencer::phaseQuestion (const float frequency, const float gain, const CalibratedSetPoint phase) const
{
    float GAIN_INCREASE = 6.0f;
    float PAN = 0.0f;
    
    float phaseValue1 = phase.estimatedValue() - phase.getWindowSize();
    float phaseValue2 = phase.estimatedValue() + phase.getWindowSize();
    
    Note note1 (frequency, gain + GAIN_INCREASE, PAN, phaseValue1);
    Note note2 (frequency, gain + GAIN_INCREASE, PAN, phaseValue2);

    return Question (QuestionType::Phase, note1, note2);
}

bool CalibrationSequencer::referenceNoteHasBeenCalibrated() const
{
    return getReferencePan().precision() < 0.01 && getReferencePhase().precision() < 0.05;
}

bool CalibrationSequencer::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointManager.amplitudesHaveBeenWindowed();
}

bool CalibrationSequencer::phasesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecisionInHzRange (PHASE_PRECISION, PHASE_LOW_HZ, PHASE_HIGH_HZ);
}

bool CalibrationSequencer::pansHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.pansHaveBeenCalibratedWithPrecision (PAN_PRECISION);
}

bool CalibrationSequencer::amplitudesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.amplitudesHaveBeenCalibratedWithPrecision (AMPLITUDE_PRECISION);
}

const Note CalibrationSequencer::referenceNote() const
{
    return Note (REFERENCE_FREQ, 
                 REFERENCE_GAIN_DB,
                 calibratedSetPointManager.getReferencePanCalibratedSetPoint().estimatedValue(),
                 calibratedSetPointManager.getReferencePhaseCalibratedSetPoint().estimatedValue());
}

const CalibratedSetPoint& CalibrationSequencer::getReferencePan() const
{
    return calibratedSetPointManager.getReferencePanCalibratedSetPoint();
}

const CalibratedSetPoint& CalibrationSequencer::getReferencePhase() const
{
    return calibratedSetPointManager.getReferencePhaseCalibratedSetPoint();
}
