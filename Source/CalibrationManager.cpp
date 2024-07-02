/*
  ==============================================================================

    CalibrationManager.cpp
    Created: 1 Jul 2024 3:34:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationManager.h"

const std::pair<float, float> CalibrationManager::getNextSample()
{
    return calibrationIntervalSequencer.getNextSample();
}

void CalibrationManager::setSampleRate (float newSampleRate)
{
    calibrationIntervalSequencer.setSampleRate (newSampleRate);
}

void CalibrationManager::calibrateWith (const CalibrationChoice choice)
{
    // TODO: Add logic to calibrate a point
}

const Curve& CalibrationManager::getCurve() const
{
    return calibratedSetPointManager.getCurve();
}

const QuestionType CalibrationManager::currentQuestionType()
{
    // TODO: Replace with actual logic not placeholder
    return QuestionType::Level;
}

const bool CalibrationManager::isPlayingFirstNote()
{
    return calibrationIntervalSequencer.isPlayingFirstNote();
}
