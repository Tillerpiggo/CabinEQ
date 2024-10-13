/*
  ==============================================================================

    BandEqCurve.cpp
    Created: 12 Oct 2024 8:26:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BandEqCurve.h"

const float BandEqCurve::dbAtFrequency (float frequency) const
{
    float dbAtFreq = 0;
    for (const auto& band : bands)
    {
        dbAtFreq += dbAtFrequencyForBand (band, frequency);
    }
    return dbAtFreq;
}

void BandEqCurve::updateWithBands (std::vector<Band> bands)
{
    this->bands = bands;
}

const float BandEqCurve::dbAtFrequencyForBand (Band band, float frequency) const
{
    // Compute frequency difference in octaves
    float x = std::log2(frequency / band.freq);
    
    // Compute denominator
    float denom = 1.0f + std::pow((2.0f * x) / band.bandwidth, 2.0f);
    
    // Calculate linear gain factor
    float K = std::pow(10.0f, band.ampl / 20.0f);
    
    // Calculate gain
    float gain = 1.0f + (K - 1.0f) / denom;
    
    // Convert gain to dB
    float db = 20.0f * std::log10(gain);

    return db;
}
