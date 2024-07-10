/*
  ==============================================================================

    CalibrationManager.cpp
    Created: 1 Jul 2024 3:34:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BinaryCalibrationManager.h"

BinaryCalibrationManager::BinaryCalibrationManager (CalibratedSetPointManager& calibratedSetPointManager)
    : calibratedSetPointManager (calibratedSetPointManager), calibrationSequencer (calibratedSetPointManager),
      currentQuestion (Question::defaultQuestion())
{
    goToNextQuestion();
    questionSequencer.setListener (this);
    calibratedSetPointManager.updateCurve (curve);
}

const std::pair<float, float> BinaryCalibrationManager::getNextSample()
{
    return questionSequencer.getNextSample();
}

void BinaryCalibrationManager::setSampleRate (float newSampleRate)
{
    questionSequencer.setSampleRate (newSampleRate);
}

void BinaryCalibrationManager::calibrateWith (const CalibrationChoice choice)
{
    calibratedSetPointManager.calibrateWith (currentQuestion, choice);
    goToNextQuestion();
}

void BinaryCalibrationManager::changeReferencePanTo (float newReferencePan)
{
    calibratedSetPointManager.changeReferencePanTo (newReferencePan);
}

void BinaryCalibrationManager::setAmplitudeAt (int index, float value)
{
    calibratedSetPointManager.setAmplitudeAt (index, value);
}

const Curve& BinaryCalibrationManager::getCurve() const
{
    return curve;
}

const Question& BinaryCalibrationManager::getCurrentQuestion() const
{
    return currentQuestion;
}

const bool BinaryCalibrationManager::isPlayingFirstNote() const
{
    // TODO: Fix isPlayingFirstNote
    return questionSequencer.isPlayingFirstNote();
}

void BinaryCalibrationManager::sequenceDidFinish()
{
    calibratedSetPointManager.calibrateWith (currentQuestion, CalibrationChoice::NoPreference);
    currentQuestion.decreaseTempo();
    questionSequencer.setQuestion (currentQuestion);
}

void BinaryCalibrationManager::goToNextQuestion()
{
    currentQuestion = calibrationSequencer.getNextQuestion();
    questionSequencer.setQuestion (currentQuestion);
}
