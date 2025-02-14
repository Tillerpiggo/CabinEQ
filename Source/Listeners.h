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
#include "Checkerboard.h"
#include "Glyph.h"

class PlaybackManagerListener
{
public:
    virtual ~PlaybackManagerListener() = default;
    
    virtual void audioFilePlayingChanged (bool isPlaying) = 0;
};

class AudioPlayerComponentListener
{
public:
    virtual ~AudioPlayerComponentListener() = default;
    
    virtual void setIsAudioFilePlaying (bool isPlaying) = 0;
    virtual void setFile (juce::File file) = 0;
    virtual void addAsListener (PlaybackManagerListener* listener) = 0;
};

class CabinPeqGraphListener
{
public:
    virtual ~CabinPeqGraphListener() = default;
    
    virtual int addMultiBandStep() = 0;
    virtual void removeMultiBandStep (int id) = 0;
    virtual void setStepEnabled (int id, bool isEnabled) = 0;
    virtual int addBand (float freq, float ampl, float bandwidth, Band::Type type, int stepId) = 0;
    virtual void updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type, int stepId) = 0;
    virtual void removeBand (int id, int stepId) = 0;
    virtual void setProfileVolume (float volume) = 0;
};

// Provides information like the BandProfile, the currently playing freq, and other useful information
class CabinPeqGraphDataSource
{
public:
    virtual ~CabinPeqGraphDataSource() = default;
    
    virtual BandProfile getBandProfile() = 0;
    virtual std::vector<float> getCurrPlayingFreqs() = 0;
    virtual std::vector<std::pair<float, float>> getCurrPlayingFreqsAndVols() = 0;
    virtual float getBandwidth() = 0;
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
    virtual void incrementGlyphVolume (int glyphId, float increment) = 0; // changes the volume of the glyph by this increment, with range 0 to 1.5
    virtual void incrementSizeFactor (int glyphId, float horizontalIncrement, float verticalIncrement) = 0; // changes the size factor of the glyph by this increment, with upper and lower bounds on size factor
    virtual void moveGlyphs (std::unordered_map<int, juce::Point<float>> idsToPositions) = 0;
    virtual void scaleGlyphs (std::unordered_set<int> glyphIds, float increment) = 0;
};

class GlyphViewDataSource
{
public:
    virtual ~GlyphViewDataSource() = default;
    
    virtual const std::vector<ArchetypalGlyph>& getArchetypalGlyphs() = 0;
    virtual const std::vector<Glyph>& getGlyphs() = 0;
    virtual float getCurrPlayingTime() = 0;
    virtual bool getIsPlaying() = 0;
    virtual float getBandwidth() = 0;
};

class CheckerboardViewListener
{
public:
    virtual ~CheckerboardViewListener() = default;
    
    virtual void setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords) = 0;
};


class CheckerboardViewDataSource
{
public:
    virtual ~CheckerboardViewDataSource() = default;
    
    virtual const Checkerboard getCheckerboard() = 0;
    virtual bool getIsPlaying() = 0;
};

class FreeTrialListener
{
public:
    virtual ~FreeTrialListener() = default;
    virtual void freeTrialDidReset() = 0;
    virtual void showActivateLicenseForm() = 0;
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
    virtual void setCalibrationVolume (float calibrationVolume) = 0;
    virtual void setIsFilterOn (bool isFilterOn) = 0;
    virtual void setIsPlaying (bool isPlaying) = 0;
    virtual void setIsCabinNoise (bool isCabinNoise) = 0;
    virtual void setMinFreq (float newMinFreq) = 0;
    virtual void setMaxFreq (float newMaxFreq) = 0;
    virtual void setSpeedFactor (float speedFactor) = 0;
    virtual void setBandwidth (float bandwidth) = 0;
    
    // This section might all be outdated now
    virtual void setIIR (bool isIIR) = 0;
    virtual void updateFIRFilter() = 0;
    virtual void setFIRQuality (int fftSize) = 0;
    
    virtual void setBarkScaling (bool barkScalingEnabled) = 0;
    virtual void setERBScaling (bool erbScalingEnabled) = 0;
    virtual void setPinkNoise (bool pinkNoiseEnabled) = 0;
    
    virtual void setIsCascading (bool isCascading) = 0;
    virtual void setDensity (int density) = 0;
    virtual void setStrokeOverlap (float strokeOverlap) = 0;
    virtual void setDotOverlap (float dotOverlap) = 0;
    virtual void setRampLength (float rampLength) = 0;
    
    // Checkerboard stuff
    virtual void setCheckerboardResolution (int newResolution) = 0;
    virtual void setCheckerboardSharpness (float newSharpness) = 0;
    virtual void toggleCheckerboardPolarity() = 0;
};


