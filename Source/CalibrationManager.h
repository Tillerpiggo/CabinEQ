/*
  ==============================================================================

    CalibrationManager.h
    Created: 1 Jul 2024 3:34:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "CalibratedSetPointManager.h"
#include "CalibrationSequencer.h"
#include "Question.h"
#include "Curve.h"
#include "CalibrationChoice.h"
#include "QuestionSequencer.h"

class CalibrationManager
{
public:
    CalibrationManager();
    
    const std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void calibrateWith (const CalibrationChoice choice);
    
    const Curve& getCurve() const;
    const Question& getCurrentQuestion() const;
    const bool isPlayingFirstNote() const;
    
private:
    void goToNextQuestion();
    
    CalibratedSetPointManager calibratedSetPointManager;
    CalibrationSequencer calibrationSequencer;
    QuestionSequencer questionSequencer;
    
    Question currentQuestion;
};
