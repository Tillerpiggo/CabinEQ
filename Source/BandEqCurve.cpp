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
    
    // Calculate exponent for the exponential function
    float ln2 = std::log(2.0f);
    float exponent = -ln2 * (4.0f * x * x) / (band.bandwidth * band.bandwidth);
    
    // Compute gain in dB directly
    float gainDB = band.ampl * std::exp(exponent);
    
    return gainDB;
}
