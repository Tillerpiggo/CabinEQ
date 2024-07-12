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

class BinaryCalibrationManager   : public SequencerListener
{
public:
    BinaryCalibrationManager();
    
    const std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    void calibrateWith (const CalibrationChoice choice);
    void changeReferencePanTo (float newReferencePan);
    
    void setAmplitudeAt (int index, float value);
    
    const Question& getCurrentQuestion() const;
    const bool isPlayingFirstNote() const;
    
    void setFletcherMunsonCompensation (float calibrationDB, float calibrationFactor, float fmDB);
    
    void sequenceDidFinish();
    const Curve& getCurve()
    {
        updateCurve();
        return curve;
    }
    
private:
    void updateCurve()
    {
        auto& frequencies = calibratedSetPointManager.getFrequencies();
        auto& amplitudePoints = calibratedSetPointManager.getAmplitudes();
        
        std::vector<float> amplitudes;
            
        for (int i = 0; i < frequencies.size(); ++i)
        {
            amplitudes.push_back (amplitudePoints.at (i).estimatedValue());
        }
        
        curve.setFrequencies (calibratedSetPointManager.getFrequencies());
        curve.setAmplitudes (amplitudes);
        curve.setPans (std::vector<float> (frequencies.size(), 0.0f));
        curve.setPhases (std::vector<float> (frequencies.size(), 0.0f));
    }
    
    void goToNextQuestion();
    
    CalibratedSetPointManager calibratedSetPointManager;
    CalibrationSequencer calibrationSequencer;
    QuestionSequencer questionSequencer;
    
    Question currentQuestion;
    Curve curve;
    
};
