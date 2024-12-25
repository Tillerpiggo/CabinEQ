/*
  ==============================================================================

    BandProfile.h
    Created: 10 Oct 2024 7:05:33pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

class Band
{
public:
    enum class Type  : int
    {
        both = 0,
        left = 1,
        right = 2
    };
    
    Band (int id, float freq, float ampl, float bandwidth, Type type)
        : id (id), freq (freq), ampl (ampl), bandwidth (bandwidth), type (type)
    {
        this->qFactor = bandwidthToQFactor (bandwidth);
    }
    
    static float bandwidthToQFactor (float bandwidth)
    {
        return std::sqrt (std::pow (2.0, bandwidth)) / (std::pow (2.0, bandwidth) - 1);
    }
    
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
    MultiBandStep (std::vector<Band> bands, bool isEnabled)
        : bands (bands), isEnabled (isEnabled)
    {}
    
    const std::vector<Band>& getBands() const
    {
        return bands;
    }
    
    bool getIsEnabled() const
    {
        return isEnabled;
    }
    
private:
    std::vector<Band> bands;
    bool isEnabled;
};

class BandProfile
{
public:
    BandProfile()
        : multiBandSteps ({}), volume (0.0f), melodyVolume (0.0f), noiseVolume (0.0f)
    {}
    
    BandProfile (std::vector<MultiBandStep> multiBandSteps, float volume, float melodyVolume, float noiseVolume)
        : multiBandSteps (multiBandSteps), volume (volume), melodyVolume (melodyVolume), noiseVolume (noiseVolume)
    {}
    
    const std::vector<MultiBandStep>& getMultiBandSteps() const
    {
        return multiBandSteps;
    }
    
    std::vector<Band> getBands()
    {
        std::vector<Band> bands;
        for (const auto& step : multiBandSteps)
        {
            bands.insert (bands.begin(), step.getBands().begin(), step.getBands().end()); // append all bands in each step
        }
        return bands;
    }
    
    const float getVolume() const
    {
        return volume;
    }
    
    const float getMelodyVolume() const
    {
        return melodyVolume;
    }
    
    const float getNoiseVolume() const
    {
        return noiseVolume;
    }
private:
    std::vector<MultiBandStep> multiBandSteps;
    float volume;
    float melodyVolume;
    float noiseVolume;
};

