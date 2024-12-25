/*
  ==============================================================================

    BandProfile.h
    Created: 10 Oct 2024 7:05:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Band
{
public:
    enum class Type  : int
    {
        both = 0,
        left = 1,
        right = 2
    };
    
    Band (int id, float freq, float ampl, float bandwidth, Type type);
    static float bandwidthToQFactor (float bandwidth);
    
    int id;
    float freq;
    float ampl;
    float bandwidth;
    float qFactor;
    Type type;
};

class MultiBandStep
{
public:
    MultiBandStep (std::vector<Band> bands, bool isEnabled);
    
    const std::vector<Band>& getBands() const;
    bool getIsEnabled() const;
    
    // BandEqCurve methods
    const float dbAtFrequency (float frequency) const;
    const float leftDbAtFrequency (float frequency) const;
    const float rightDbAtFrequency (float frequency) const;
    const float dbAtFrequencyForBand (Band band, float frequency) const;
    
private:
    std::vector<Band> bands;
    bool isEnabled;
};

class BandProfile
{
public:
    BandProfile();
    BandProfile (std::vector<MultiBandStep> multiBandSteps, float volume, float melodyVolume, float noiseVolume);
    
    const std::vector<MultiBandStep>& getMultiBandSteps() const;
    std::vector<Band> getBands();
    const float getVolume() const;
    const float getMelodyVolume() const;
    const float getNoiseVolume() const;
private:
    std::vector<MultiBandStep> multiBandSteps;
    float volume;
    float melodyVolume;
    float noiseVolume;
};

