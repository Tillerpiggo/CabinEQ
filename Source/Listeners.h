/*
  ==============================================================================

    Listeners.h
    Created: 18 Nov 2024 11:26:17am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"

class CabinPeqGraphListener
{
public:
    virtual ~CabinPeqGraphListener() = default;
    
    virtual int addBand (float freq, float ampl, float bandwidth, Band::Type type) = 0;
    virtual void updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type) = 0;
    virtual void removeBand (int id) = 0;
    virtual void setProfileVolume (float volume) = 0;
};

// Provides information like the BandProfile, the currently playing freq, and other useful information
class CabinPeqGraphDataSource
{
public:
    virtual ~CabinPeqGraphDataSource() = default;
    
    virtual BandProfile getBandProfile() = 0;
};
