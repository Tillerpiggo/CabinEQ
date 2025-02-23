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
#include "Checkerboard.h"

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

class CheckerboardViewListener
{
public:
    virtual ~CheckerboardViewListener() = default;
    
    virtual void setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords) = 0;
};

class MusicListListener
{
public:
    virtual ~MusicListListener() = default;
    
    virtual void selectedRow (int rowIdx) = 0;
};

class ProfileViewListener
{
public:
    virtual ~ProfileViewListener() = default;
    
    virtual void addProfile (juce::String profileName) = 0;
    virtual void addDuplicateProfile (juce::String profileName, juce::String oldProfileName) = 0;
    virtual void renameProfile (juce::String profileName, juce::String newProfileName) = 0;
    virtual void deleteProfile (juce::String profileName) = 0;
    virtual void selectProfile (juce::String profileName) = 0;
};

class ProfileViewDataSource
{
public:
    virtual ~ProfileViewDataSource() = default;
    
    virtual std::vector<juce::String> getProfileNames() = 0;
    virtual bool getIsProfileLocked (int rowIdx) = 0;
};

class ProfileListListener
{
public:
    virtual ~ProfileListListener() = default;
    
    virtual void selectedRow (int rowIdx) = 0;
    virtual void duplicateProfile (int rowIdx) = 0;
    virtual void renameProfile (int rowIdx, juce::String newProfileName) = 0;
    virtual void deleteProfile (int rowIdx) = 0;
    virtual void addProfile (juce::String profileName) = 0;
};

class ProfileListDataSource
{
public:
    virtual ~ProfileListDataSource() = default;
    
    virtual std::vector<juce::String> getProfileNames() = 0;
    virtual bool getIsProfileLocked (int rowIdx) = 0;
};

class CheckerboardViewDataSource
{
public:
    virtual ~CheckerboardViewDataSource() = default;
    
    virtual const Checkerboard getCheckerboard() = 0;
    virtual bool getIsPlaying() = 0;
    virtual int getNumCheckerboards() = 0;
    virtual std::string getNameAtIdx (int idx) = 0;
    virtual int getSelectedRow() = 0;
};

class FreeTrialListener
{
public:
    virtual ~FreeTrialListener() = default;
    virtual void freeTrialDidReset() = 0;
    virtual void showActivateLicenseForm() = 0;
};

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
    
    // This section might all be outdated now
    virtual void setIIR (bool isIIR) = 0;
    virtual void updateFIRFilter() = 0;
    virtual void setFIRQuality (int fftSize) = 0;
    
    virtual void setPinkNoise (bool pinkNoiseEnabled) = 0;
    
    // Checkerboard stuff
    virtual void goToNext() = 0;
    virtual void goToPrev() = 0;
    virtual bool hasNext() = 0;
    virtual bool hasPrev() = 0;
    virtual void toggleCheckerboardPolarity() = 0;
    
    virtual void selectCheckerboardAtIdx (int idx) = 0;
};

class ContactUsBannerListener
{
public:
    virtual ~ContactUsBannerListener() = default;
    
    virtual void restartAudio() = 0;
};

class ElevationCalibrationListener
{
public:
    virtual ~ElevationCalibrationListener() = default;
    
    virtual void selectedRowChanged(int newRow) = 0;
    virtual void increaseNumRows() = 0;
    virtual void decreaseNumRows() = 0;
};

class ElevationCalibrationDataSource
{
public:
    virtual ~ElevationCalibrationDataSource() = default;
    
    virtual int getNumRows() = 0;
    virtual int getSelectedElevationRow() = 0;
    virtual int getPlayingElevationRow() = 0;
    virtual bool getIsPlaying() = 0;
    virtual bool canIncreaseNumRows() = 0;
    virtual bool canDecreaseNumRows() = 0;
};


