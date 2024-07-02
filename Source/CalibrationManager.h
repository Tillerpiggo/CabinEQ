/*
  ==============================================================================

    CalibrationManager.h
    Created: 1 Jul 2024 3:34:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibratedSetPointManager.h"
#include "CalibrationIntervalSequencer.h"
#include "QuestionType.h"
#include "Curve.h"
#include "CalibrationChoice.h"

class CalibrationManager
{
public:
    CalibrationManager() {}
    
    const std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void calibrateWith (const CalibrationChoice choice);
    
    const Curve& getCurve() const;
    const QuestionType currentQuestionType();
    const bool isPlayingFirstNote();
    
private:
    CalibratedSetPointManager calibratedSetPointManager;
    CalibrationIntervalSequencer calibrationIntervalSequencer;
};
