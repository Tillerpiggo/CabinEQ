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
#include "Glyph.h"

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
    virtual std::vector<float> getCurrPlayingFreqs() = 0;
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
    virtual float getCurrTime() = 0;
};

class GlyphViewListener
{
public:
    virtual ~GlyphViewListener() = default;
        
    virtual void addGlyph (ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor) = 0;
    virtual void moveGlyph (int glyphId, juce::Point<float> centerPos) = 0; // tries to move the glyph, although bounds will be applied
    virtual void removeGlyph (int glyphId) = 0; // removes the glyph if the glyph is in the list of glyphs. If there are multiple with this id, only removes the first one, although this should never happen
};

class GlyphViewDataSource
{
public:
    virtual ~GlyphViewDataSource() = default;
    
    virtual const std::vector<ArchetypalGlyph>& getArchetypalGlyphs() = 0;
    virtual const std::vector<Glyph>& getGlyphs() = 0;
    virtual float getCurrPlayingTime() = 0;
};

//class GlyphViewDataSource
//{
//public:
//    virtual ~GlyphViewDataSource() = default;
//    
//    virtual Glyph getCurrGlyph() = 0;
//    virtual bool hasNextGlyph() = 0;
//    virtual bool hasPrevGlyph() = 0;
//    
//    virtual float getSizeFactor() = 0;
//    virtual juce::Point<float> getCenterPos() = 0;
//    
//    virtual float getCurrPlayingTime() = 0;
//};

class CalibrationListener
{
public:
    virtual ~CalibrationListener() = default;
    
    virtual void setVolume (float volume) = 0;
    virtual void setIsFilterOn (bool isFilterOn) = 0;
    virtual void setIsPlaying (bool isPlaying) = 0;
    virtual void setIsCabinNoise (bool isCabinNoise) = 0;
    virtual void setMinFreq (float newMinFreq) = 0;
    virtual void setMaxFreq (float newMaxFreq) = 0;
    virtual void setSpeedFactor (float speedFactor) = 0;
    virtual void setBandwidth (float bandwidth) = 0;
};
