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
    int getNumLockedIn();
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
    
    // Forced perfectionism
    int goToNextQuestion (float newVal);
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
    
    int referenceToneIdx = 5;
    int currIdx = -1;
    
    int noteLength = 20000;
    
    std::vector<int> comparisonList = { 5, 4, 6, 3, 7, 2, 8, 1, 9, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
    
    // Forced perfectionism
    int getRandomElement (std::vector<int> vec) const;
    int removeElementMatching (int val, std::vector<int>& vec);
    
    std::vector<int> lockedInIndices;
    std::vector<int> pendingIndices = { 4, 5 };
    int currNoteIdx = 4; // as an absolute index
    int nextNoteIdx = 2; // in the comparison list
    
    int lastAskedIdx = -1;
};
