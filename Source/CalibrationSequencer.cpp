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
: calibratedSetPointManager (calibratedSetPointManager),
  referencePan (REFERENCE_PAN_WINDOW), referencePhase (REFERENCE_PHASE_WINDOW)
{
    
}

Question CalibrationSequencer::getNextQuestion()
{
    // TODO: Figure out what state we need to track to implement a basic calibration sequence
    // For now, just repeatedly ask for panning, then phase, then level of the 11 notes
    if (! referenceNoteHasBeenCalibrated())
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution dist(0.5);
        bool coinFlip = dist(gen);
        
        // Calibrate pan or phase (choose randomly)
        if (coinFlip)
        {
            Note note1 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() - referencePan.getWindowSize(),
                        0.0f);
            
            Note note2 (REFERENCE_GAIN_DB,
                        REFERENCE_FREQ,
                        referencePan.estimatedValue() + referencePan.getWindowSize(),
                        0.0f);
            
            return Question (QuestionType::Pan, note1, note2);
        }
        else
        {
            return phaseQuestion (REFERENCE_FREQ, REFERENCE_GAIN_DB, referencePhase);
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
        auto [freq, phaseCalibratedSetPoint] = calibratedSetPointManager.getRandomLowestPrecisionPhase();
        Question phaseQuestionVar = phaseQuestion (freq, calibratedSetPointManager.amplitudeAt (freq), phaseCalibratedSetPoint);
        std::cout << "Phase question >" << std::endl;
        std::cout << "Note1: freq = " << phaseQuestionVar.getNote1().frequency
        << ", phase = " << phaseQuestionVar.getNote1().phase << std::endl;
        std::cout << "Note2: freq = " << phaseQuestionVar.getNote2().frequency
        << ", phase = " << phaseQuestionVar.getNote2().phase << std::endl;
        
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
    
    std::cout << "phase window size: " << phase.getWindowSize() << std::endl;
    
    float phaseValue1 = phase.estimatedValue() - phase.getWindowSize();
    float phaseValue2 = phase.estimatedValue() + phase.getWindowSize();
    
    Note note1 (frequency, gain + GAIN_INCREASE, PAN, phaseValue1);
    Note note2 (frequency, gain + GAIN_INCREASE, PAN, phaseValue2);

    return Question (QuestionType::Phase, note1, note2);
}

bool CalibrationSequencer::referenceNoteHasBeenCalibrated() const
{
    return referencePan.precision() < 0.02 && referencePhase.precision() < 0.01;
}

bool CalibrationSequencer::amplitudesHaveBeenWindowed() const
{
    return calibratedSetPointManager.amplitudesHaveBeenWindowed();
}

bool CalibrationSequencer::phasesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.phasesHaveBeenCalibratedWithPrecision (PHASE_PRECISION);
}

bool CalibrationSequencer::pansHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.pansHaveBeenCalibratedWithPrecision (PAN_PRECISION);
}

bool CalibrationSequencer::amplitudesHaveBeenCalibratedPrecisely() const
{
    return calibratedSetPointManager.amplitudesHaveBeenCalibratedWithPrecision (AMPLITUDE_PRECISION);
}

Note CalibrationSequencer::referenceNote()
{
    return Note (REFERENCE_FREQ, 
                 REFERENCE_GAIN_DB,
                 referencePan.estimatedValue(), 
                 referencePhase.estimatedValue());
}
