/*
  ==============================================================================

    PitchedGenerator.h
    Created: 12 Aug 2024 9:08:48pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Note.h"

// Abstract class whose descendants can be used to generate sounds at a specific frequency, volume, pan, and phase.
class PitchedGenerator
{
public:
    virtual ~PitchedGenerator() = default;
    
    virtual const std::pair<float, float> getNextSample() = 0;
    virtual void setSampleRate (float newSampleRate) = 0;
    
    virtual void setNote (Note note) = 0; // changes to a new note, triggering the "start" of the new note
    virtual void setFrequency (float frequencyInHz) = 0; // changes the frequency of the currently playing note
    virtual void setVolume (float volumeInDecibels) = 0; // changes the volume of the currently playing note
    virtual void setCrossfeedGain (float crossfeedGain) = 0;
    virtual void setCrossfeedDelayInMs (float delayInMs) = 0;
    virtual void setCrossfeedChannel (Channel channel) = 0;
};
