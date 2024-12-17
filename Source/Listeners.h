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
#include "NoiseSequence.h"
#include "NoiseSequenceGrid.h"

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

class NoiseGridViewListener
{
public:
    virtual ~NoiseGridViewListener() = default;
    
    virtual void addSequence (NoiseSequence sequence) = 0;
    virtual void addSequenceWithCoords (std::vector<std::pair<int, int>> coords) = 0;
    virtual void removeSequence (std::pair<int, int> origin) = 0;
    virtual void moveSequence (int id, std::pair<int, int> newOrigin) = 0;
    virtual void toggleCoords (std::pair<int, int> point) = 0;
    
    virtual void scaleUpGrid() = 0;
    virtual void scaleDownGrid() = 0;
};

class NoiseGridViewDataSource
{
public:
    virtual ~NoiseGridViewDataSource() = default;
    
    virtual NoiseSequenceGrid getNoiseGrid() = 0;
    virtual std::pair<int, int> getNumRowsAndNumCols() = 0;
    virtual int getSequenceIdAtCoords (std::pair<int, int> coords) = 0;
};
