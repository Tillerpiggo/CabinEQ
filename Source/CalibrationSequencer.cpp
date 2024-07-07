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
    std::vector<Sequence> sequences = { 
        Sequence (SequenceType::INIT_AMPLITUDE_WINDOWS, 0),
        Sequence (SequenceType::PAN, 6.0f),
        Sequence (SequenceType::AMPLITUDE, 12.0f),
        Sequence (SequenceType::PAN, 3.0f),
        Sequence (SequenceType::AMPLITUDE, 6.0f),
        Sequence (SequenceType::PAN, 1.5f),
        Sequence (SequenceType::AMPLITUDE, 3.0f),
        Sequence (SequenceType::PAN, 0.75f),
        Sequence (SequenceType::AMPLITUDE, 1.5f),
        Sequence (SequenceType::PAN, 0.25f),
        Sequence (SequenceType::AMPLITUDE, 0.5f),
        Sequence (SequenceType::PHASE, 0.01f)
    };
    
    for (auto& sequence : sequences)
    {
        if (! hasSequenceCompleted (sequence))
        {
            return executeSequence (sequence);
        }
    }
    
    return Question::defaultQuestion();
}

bool CalibrationSequencer::hasSequenceCompleted (Sequence sequence)
{
    switch (sequence.type)
    {
        case SequenceType::INIT_AMPLITUDE_WINDOWS:
            return amplitudesHaveBeenWindowed();
        case SequenceType::AMPLITUDE:
            return amplitudesHaveBeenCalibratedWithPrecision (sequence.precision);
        case SequenceType::PAN:
            return pansHaveBeenCalibratedWithPrecision (sequence.precision);
        case SequenceType::PHASE:
            return phasesHaveBeenCalibratedWithPrecision (sequence.precision);
    }
}

Question CalibrationSequencer::executeSequence(Sequence sequence)
{
    switch (sequence.type)
    {
        case SequenceType::INIT_AMPLITUDE_WINDOWS:
        {
            auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
            
            Note note1 = referenceNote();
            Note note2 (freq,
                        amplitudeCalibratedSetPoint.estimatedValue(),
                        0.0f,
                        0.0f);
            
            QuestionType type = amplitudesHaveUpperBounds() ? QuestionType::LowerThan :
                                                              QuestionType::HigherThan;
            
            return Question (type, note1, note2);
        }
        case SequenceType::AMPLITUDE:
        {
            auto [freq, amplitudeCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionAmplitude();
            
            Note note1 = referenceNote();
            Note note2 (freq,
                        amplitudeCalibratedSetPoint.estimatedValue(),
                        calibratedSetPointManager.panAt (freq),
                        calibratedSetPointManager.phaseAt (freq));
            
            return Question (QuestionType::Level, note1, note2);
        }
        case SequenceType::PAN:
        {
            auto [freq, panCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPan();
            
            Note note1 = referenceNote();
            Note note2 (freq,
                        calibratedSetPointManager.amplitudeAt (freq),
                        panCalibratedSetPoint.estimatedValue(),
                        calibratedSetPointManager.phaseAt (freq));
            
            return Question (QuestionType::Pan, note1, note2);
        }
        case SequenceType::PHASE:
        {
            auto [freq, phaseCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPhaseInRange (PHASE_LOW_HZ, PHASE_HIGH_HZ);
            
            Note note1 (freq,
                        calibratedSetPointManager.amplitudeAt (freq),
                        calibratedSetPointManager.panAt (freq),
                        phaseCalibratedSetPoint.getCurrentGuess());
            Note note2 (freq,
                        calibratedSetPointManager.amplitudeAt (freq),
                        calibratedSetPointManager.panAt (freq),
                        phaseCalibratedSetPoint.getNextGuess());
            
            return Question (QuestionType::Phase, note1, note2);
        }
    }
}

Question CalibrationSequencer::phaseQuestion (const float frequency, const float gain, const PhaseCalibratedSetPoint phase) const
{
    float GAIN_INCREASE = 6.0f;
    float PAN = 0.0f;
    
    Note note1 (frequency, gain + GAIN_INCREASE, PAN, phase.getCurrentGuess());
    Note note2 (frequency, gain + GAIN_INCREASE, PAN, phase.getNextGuess());
    
    return Question (QuestionType::Phase, note1, note2);
}

bool CalibrationSequencer::referenceNoteHasBeenCalibrated() const
{
    //return getReferencePan().precision() < 0.01 && getReferencePhase().precision() < 0.05;
    return getReferencePhase().precision() < 0.05;
}

bool CalibrationSequencer::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointManager.amplitudesHaveBeenWindowed();
}

bool CalibrationSequencer::amplitudesHaveUpperBounds() const
{
    return calibratedSetPointManager.amplitudesHaveUpperBounds();
}

bool CalibrationSequencer::phasesHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecisionInHzRange (precision, PHASE_LOW_HZ, PHASE_HIGH_HZ);
}

bool CalibrationSequencer::pansHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointManager.pansHaveBeenCalibratedWithPrecision (precision);
}

bool CalibrationSequencer::amplitudesHaveBeenCalibratedWithPrecision (float precision) const
{
    return calibratedSetPointManager.amplitudesHaveBeenCalibratedWithPrecision (precision);
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
