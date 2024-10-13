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
    float db = 1.0f + (std::pow (10.0f, band.ampl / 20.0f) - 1.0f) * 1.0f / (1.0f + ((frequency - band.freq) / (band.bandwidth / 2.0f)));
    return db;
}
