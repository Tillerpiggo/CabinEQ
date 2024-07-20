/*
  ==============================================================================

    SliderCalibrationManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SliderSequencer.h"
#include "SliderSetPointManager.h"
#include "ArbitraryResponseFilter.h"
#include "Curve.h"
#include <random>

class SliderCalibrationManager
{
public:
    SliderCalibrationManager();

    const Curve& getCurve();

    std::pair<float, float> getNextSample();
    int getCurrentlyPlayingIdx();
    bool getIsCalibrating() const;
    
    const std::vector<int>& getTuningIndices();

    void setSampleRate (float newSampleRate);
    void setCurrIdx (int idx);
    void setAmplitudeAtIdx (int idx, float newAmplitude);
    void setPanAtIdx (int idx, float newPan);
    void setIsCalibrating (bool isCalibrating);
    
    void incrementReferenceToneIndex();
    void decrementReferenceToneIndex();
    void changeNoteLength (int newNoteLength);
    
    int goToNextBlindQuestion();
private:
    void updateCurve();
    
    void updateReferenceTone();
    
    SliderSequencer sliderSequencer;
    SliderSetPointManager sliderSetPointManager;
    Curve curve;
    bool isCalibrating;
    
    int referenceToneIdx = 5;
    int currIdx = -1;
    
    int noteLength = 20000;
    
    // Blind calibration
    int currBlindIdx = 0;
};
