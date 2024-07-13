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
    int getCurrentlyPlayingTuningIdx();
    bool getIsCalibrating() const;
    
    const std::vector<float>& getTuningIndices();

    void setSampleRate (float newSampleRate);
    void setCurrIdx (int idx);
    void setAmplitudeAtIdx (int idx, float newAmplitude);
    void setPanAtIdx (int idx, float newPan);
    void setIsCalibrating (bool isCalibrating);
    void changeTuningIndices();

private:
    void updateCurve();
    void playTuningNotes();
    
    SliderSequencer sliderSequencer;
    SliderSetPointManager sliderSetPointManager;
    Curve curve;
    bool isCalibrating;
    
    std::vector<float> tuningIndices;
    std::map<float, int> tuningIndexCounts;
};
