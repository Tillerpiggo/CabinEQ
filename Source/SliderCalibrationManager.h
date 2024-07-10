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
    SliderCalibrationManager() {};
    
    void updateFilter (ArbitraryResponseFilter& filter);
    
    std::pair<float, float> getNextSample()
    {
        return sliderSequencer.getNextSample();
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
    }
    
private:
    SliderSequencer sliderSequencer;
    SliderSetPointManager sliderSetPointManager;
    Curve curve;
};
