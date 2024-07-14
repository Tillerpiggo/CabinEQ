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
    
    // Forced perfectionism
    int goToNextQuestion();
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
    
    std::vector<int> comparisonList = { 5, 4, 6, 3, 7, 2, 8, 1, 9, 0 };
    
    // Forced perfectionism
    int getRandomElement (std::vector<int> vec) const;
    int removeElementMatching (int val, std::vector<int>& vec);
    
    std::vector<int> lockedInIndices = { 5 };
    std::vector<int> pendingIndices = { 4, 6 };
    int currNoteIdx = 5; // as an absolute index
    int nextNoteIdx = 3; // in the comparison list
    float lastVal = -48.0f; // junk value that forces large diff
};
