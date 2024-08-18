/*
  ==============================================================================

    Note.h
    Created: 20 Jun 2024 12:31:21pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Channel.h"

class Note
{
public:
    Note (float frequency, float gain, float crossfeedGain = 0.0f, float crossfeedDelayInMs = 6.0f, float crossfeedChannel = Channel::CENTER)
        : frequency (frequency), gain (gain), crossfeedGain (crossfeedGain), crossfeedDelayInMs (crossfeedDelayInMs), crossfeedChannel (crossfeedChannel)
    {}
    
    Note withLeftCrossfeed (float crossfeedGain = 0.3f, float crossfeedDelayInMs = 6.0f)
    {
        return Note (frequency, gain, crossfeedGain, crossfeedDelayInMs, Channel::LEFT);
    }
    
    Note withRightCrossfeed (float crossfeedGain = 0.3f, float crossfeedDelayInMs = 6.0f)
    {
        return Note (frequency, gain, crossfeedGain, crossfeedDelayInMs, Channel::RIGHT);
    }
    
    float frequency; // hz
    float gain; // in dB
    
    // For crossfeed
    float crossfeedGain;
    float crossfeedDelayInMs;
    Channel crossfeedChannel;
};
