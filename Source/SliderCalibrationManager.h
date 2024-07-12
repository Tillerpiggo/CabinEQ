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

class SliderCalibrationManager
{
public:
    SliderCalibrationManager() : isSlidingSlider (false)
    {
        sliderSequencer.playRandomNotes (sliderSetPointManager.getFrequencies(), 
                                         sliderSetPointManager.getAmplitudes());
    }
    
    void updateFilter (ArbitraryResponseFilter& filter);
    
    std::pair<float, float> getNextSample()
    {
        return sliderSequencer.getNextSample();
    }
    
    int getCurrentlyPlayingIdx()
    {
        return sliderSetPointManager.indexForFrequency (sliderSequencer.currentlyPlayingFrequency());
    }
    
    bool getIsSlidingSlider() const
    {
        return isSlidingSlider;
    }
    
    void setSampleRate (float newSampleRate)
    {
        sliderSequencer.setSampleRate (newSampleRate);
    }
    
    void setCurrIdx (int idx)
    {
        sliderSequencer.playInterval (sliderSetPointManager.getFrequencyAt (idx),
                                      sliderSetPointManager.getAmplitudeAt (idx));
        
    }
    
    void setAmplitudeAtIdx (int idx, float newAmplitude)
    {
        sliderSetPointManager.setAmplitudeAt (idx, newAmplitude);
//        sliderSequencer.changeControlledAmplitude (newAmplitude);
        sliderSequencer.changeAmplitudeOfNotesWithFrequency (sliderSetPointManager.getFrequencyAt (idx),
                                                             newAmplitude);
    }
    
    void setIsSlidingSlider (bool isSliding)
    {
        isSlidingSlider = isSliding;
    }
    
private:
    SliderSequencer sliderSequencer;
    SliderSetPointManager sliderSetPointManager;
    Curve curve;
    bool isSlidingSlider;
};
