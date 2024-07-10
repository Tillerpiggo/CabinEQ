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
    SliderSequencer() {};
    
    std::pair<float, float> getNextSample() { return arbitrarySequencer.getNextSample(); }
    void setSampleRate (float newSampleRate) { arbitrarySequencer.setSampleRate (newSampleRate); }
    
    void playInterval (float frequency, float amplitude);
    
private:
    ArbitrarySequencer arbitrarySequencer;
    Note referenceNote = Note (1000.0f, 12.0f, 0.0f, 0.0f);
};
