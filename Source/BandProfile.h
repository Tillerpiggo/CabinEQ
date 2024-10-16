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
    Band (int id, float freq, float ampl, float bandwidth)
        : id (id), freq (freq), ampl (ampl), bandwidth (bandwidth)
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
};

class BandProfile
{
public:
    BandProfile()
        : bands ({}), volume (0.0f)
    {}
    
    BandProfile (std::vector<Band> bands, float volume)
        : bands (bands), volume (volume)
    {}
    
    const std::vector<Band>& getBands() const
    {
        return bands;
    }
    
    const float getVolume() const
    {
        return volume;
    }
private:
    std::vector<Band> bands;
    float volume;
};

