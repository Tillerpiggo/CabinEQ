/*
  ==============================================================================

    SliderSequencer.h
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CalibratedSetPointManager.h"
#include "ArbitrarySequencer.h"
#include "Note.h"

class SliderSequencer
{
public:
    SliderSequencer (const CalibratedSetPointManager& calibratedSetPointManager);
    
    std::pair<float, float> getNextSample();
    void setSampleRate (float newSampleRate);
    
    void playIntervalAtIdx (int idx); // start playing the interval at the given index
    
private:
    ArbitrarySequencer arbitrarySequencer;
    Note referenceNote = Note (1000.0f, 12.0f, 0.0f, 0.0f);
    
    const CalibratedSetPointManager& calibratedSetPointManager;
    
};
