/*
  ==============================================================================

    SliderSequencer.h
    Created: 10 Jul 2024 1:05:40pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ArbitrarySequencer.h"
#include "Note.h"
#include "EQNode.h"

class SliderSequencer
{
public:
    SliderSequencer() {};
    
    void playInterval (float frequency, float amplitude, float pan, int noteLength, bool repeating);
    void playInterval (EQNode eqNode, int noteLength, bool repeating);
    void changeControlledAmplitude (float newAmplitude);
    void changeControlledPan (float newPan);
    
    void changeAmplitudeOfNotesWithFrequency (float frequency, float newAmplitude);
    void changePanOfNotesWithFrequency (float frequency, float newPan);
    
    std::pair<float, float> getNextSample();
    float currentlyPlayingFrequency() const;
    void setSampleRate (float newSampleRate);
    
private:
    ArbitrarySequencer arbitrarySequencer;
    Note referenceNote = Note (1000.0f, 6.0f, 0.0f, 0.0f);
};
