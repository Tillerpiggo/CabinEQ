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
    {}
    
    int id;
    float freq;
    float ampl;
    float bandwidth;
};

class BandProfile
{
public:
    BandProfile() {}
    
    const std::vector<Band>& getBands()
    {
        return bands;
    }
private:
    std::vector<Band> bands;
};

