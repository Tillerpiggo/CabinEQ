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
    float getCurrBlindPercent() const;
    
    const std::vector<int>& getTuningIndices();

    void setSampleRate (float newSampleRate);
    void setCurrIdx (int idx);
    void setAmplitudeAtIdx (int idx, float newAmplitude);
    std::pair<int, float> incrementAmplitudeAtBlindIdxAndPlay (float increment);
    int setAmplitudeAtBlindIdx (float newAmplitude);
    void setPanAtIdx (int idx, float newPan);
    void setIsCalibrating (bool isCalibrating);
    
    void incrementReferenceToneIndex();
    void decrementReferenceToneIndex();
    void changeNoteLength (int newNoteLength);
    
    int goToNextBlindQuestion();
    void playBlindInterval();
    
    // Testing
    
    // Getters
    bool getIsTesting() const;
    bool getIsFilterEnabled() const;
    float getFilterGain() const;

    // Setters
    void setIsTesting (bool isTesting);
    void setIsFilterEnabled (bool isFilterEnabled);
    
    void setTone1Freq (float tone1Freq);
    void setTone2Freq (float tone2Freq);
    void setTone1Vol (float tone1Vol);
    void setTone2Vol (float tone2Vol);
    void setFilterGain (float filterGain);
    
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
    std::vector<float> randomIndices;
    
    // Testing (verifying the filter)
    bool isTesting = false;
    bool isFilterEnabled = false;
    
    float tone1Freq = 1000.0f;
    float tone2Freq = 2000.0f;
    float tone1Vol = 6.0f;
    float tone2Vol = 6.0f;
    float filterGain = 0.0f;
    
};
