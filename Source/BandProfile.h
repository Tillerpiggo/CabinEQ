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
        double Q = std::sqrt (std::pow (2.0, bandwidth)) / (std::pow (2.0, bandwidth) - 1);
        this->qFactor = Q;
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
    BandProfile() {}
    
    const std::vector<Band>& getBands()
    {
        return bands;
    }
private:
    std::vector<Band> bands;
};

