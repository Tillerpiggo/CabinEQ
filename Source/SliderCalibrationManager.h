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
    
    const std::vector<int>& getTuningIndices();

    void setSampleRate (float newSampleRate);
    void setCurrIdx (int idx);
    void setAmplitudeAtIdx (int idx, float newAmplitude);
    void setPanAtIdx (int idx, float newPan);
    void setIsCalibrating (bool isCalibrating);
    void changeTuningIndices();
    
    void incrementReferenceToneIndex();
    void decrementReferenceToneIndex();
    void changeNoteLength (int newNoteLength);

private:
    void updateCurve();
    void playTuningNotes();
    std::vector<int> generateTuningPattern (int start, int end, int octave);
    
    void updateReferenceTone();
    
    SliderSequencer sliderSequencer;
    SliderSetPointManager sliderSetPointManager;
    Curve curve;
    bool isCalibrating;
    
    std::vector<int> tuningIndices;
    std::map<float, int> tuningIndexCounts;
    
    //std::vector<float> octaveOrder = { 5, 4, 6, 3, 7, 2, 8, 1, 9, 0 };
//    int octaveIdx = 0;
    
    int referenceToneIdx = 5;
    int currIdx = -1;
    
    int noteLength = 20000;
    
    std::vector<int> comparisonList = { 5, 4, 6, 3, 7, 2, 8, 1, 9, 0 };
};
