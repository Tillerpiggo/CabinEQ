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
#include "SetPointManager.h"
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
    
    void setSampleRate (float newSampleRate);
    void setCurrIdx (int idx);
    void setAmplitudeAtIdx (int idx, float newAmplitude);
    void setPanAtIdx (int idx, float newPan);
    void setIsCalibrating (bool isCalibrating);
    
    void incrementReferenceToneIndex();
    void decrementReferenceToneIndex();
    void changeNoteLength (int newNoteLength);
    
    int goToNextBlindQuestion();
    void playBlindInterval();
    
private:
    void updateCurve();
    
    void updateReferenceTone();
    
    SliderSequencer sliderSequencer;
    SetPointManager setPointManager;
    Curve curve;
    bool isCalibrating;
    
    int currIdx = -1;
    
    int noteLength = 25000;
};
