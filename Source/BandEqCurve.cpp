/*
  ==============================================================================

    BandEqCurve.cpp
    Created: 12 Oct 2024 8:26:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BandEqCurve.h"

void BandEqCurve::updateWithBands (const std::vector<Band>& newBands)
{
    bands = newBands;
    recalculate();
}

void BandEqCurve::setSampleRate (double newSampleRate)
{
    if (newSampleRate > 0.0 && newSampleRate != sampleRate)
    {
        sampleRate = newSampleRate;
        recalculate();
    }
}

void BandEqCurve::recalculate()
{
    cachedBands.clear();
    for (const auto& band : bands)
        if (band.enabled)
            cachedBands.push_back ({ band, FilterDesign::design (band, sampleRate) });
}

float BandEqCurve::dbAtFrequency (float frequency) const
{
    if (! hasChannelSpecificBands())
        return leftDbAtFrequency (frequency);
    return 0.5f * (leftDbAtFrequency (frequency) + rightDbAtFrequency (frequency));
}

float BandEqCurve::leftDbAtFrequency (float frequency) const
{
    return dbAtFrequencyForChannel (frequency, 0);
}

float BandEqCurve::rightDbAtFrequency (float frequency) const
{
    return dbAtFrequencyForChannel (frequency, 1);
}

float BandEqCurve::dbAtFrequencyForChannel (float frequency, int channel) const
{
    if (frequency >= sampleRate * 0.5)
        return 0.0f;

    double db = 0.0;
    for (const auto& cached : cachedBands)
        if (cached.band.appliesToChannel (channel))
            db += FilterDesign::magnitudeDb (cached.coefficients, frequency, sampleRate);
    return (float) db;
}

float BandEqCurve::dbAtFrequencyForBand (const Band& band, float frequency) const
{
    if (frequency >= sampleRate * 0.5)
        return 0.0f;
    return (float) FilterDesign::magnitudeDb (FilterDesign::design (band, sampleRate), frequency, sampleRate);
}

bool BandEqCurve::hasChannelSpecificBands() const
{
    for (const auto& cached : cachedBands)
        if (cached.band.type != Band::Type::both)
            return true;
    return false;
}

float BandEqCurve::loudnessChangeDb() const
{
    if (cachedBands.empty())
        return 0.0f;

    // Average the power gain over log-spaced frequencies, weighted towards where music has
    // most of its perceived loudness: flat from 100 Hz to 5 kHz, fading out to 20 Hz and 16 kHz.
    constexpr int numPoints = 256;
    const double logLow = std::log (20.0), logHigh = std::log (16000.0);
    const double logFullLow = std::log (100.0), logFullHigh = std::log (5000.0);

    double totalPower[2] = { 0.0, 0.0 };
    double totalWeight = 0.0;

    for (int i = 0; i < numPoints; ++i)
    {
        const double logFreq = logLow + (logHigh - logLow) * (i + 0.5) / numPoints;
        double weight = 1.0;
        if (logFreq < logFullLow)
            weight = (logFreq - logLow) / (logFullLow - logLow);
        else if (logFreq > logFullHigh)
            weight = (logHigh - logFreq) / (logHigh - logFullHigh);

        const auto freq = (float) std::exp (logFreq);
        for (int channel = 0; channel < 2; ++channel)
            totalPower[channel] += weight * std::pow (10.0, dbAtFrequencyForChannel (freq, channel) / 10.0);
        totalWeight += weight;
    }

    const double leftDb = 10.0 * std::log10 (totalPower[0] / totalWeight);
    const double rightDb = 10.0 * std::log10 (totalPower[1] / totalWeight);
    return (float) (0.5 * (leftDb + rightDb));
}
