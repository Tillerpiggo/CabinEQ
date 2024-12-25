/*
  ==============================================================================

    BandProfile.cpp
    Created: 25 Dec 2024 2:43:35pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "BandProfile.h"

//==============================================
Band::Band (int id, float freq, float ampl, float bandwidth, Type type)
: id (id), freq (freq), ampl (ampl), bandwidth (bandwidth), type (type)
{
    this->qFactor = bandwidthToQFactor (bandwidth);
}

float Band::bandwidthToQFactor (float bandwidth)
{
    return std::sqrt (std::pow (2.0, bandwidth)) / (std::pow (2.0, bandwidth) - 1);
}

//==============================================
MultiBandStep::MultiBandStep (std::vector<Band> bands, bool isEnabled)
    : bands (bands), isEnabled (isEnabled)
{}

const std::vector<Band>& MultiBandStep::getBands() const
{
    return bands;
}

bool MultiBandStep::getIsEnabled() const
{
    return isEnabled;
}

const float MultiBandStep::dbAtFrequency (float frequency) const
{
    float dbAtFreq = 0;
    for (const auto& band : bands)
    {
        dbAtFreq += dbAtFrequencyForBand (band, frequency);
    }
    return dbAtFreq;
}

const float MultiBandStep::leftDbAtFrequency (float frequency) const
{
    float dbAtFreq = 0;
    for (const auto& band : bands)
    {
        if (band.type == Band::Type::both || band.type == Band::Type::left)
        {
            dbAtFreq += dbAtFrequencyForBand (band, frequency);
        }
    }
    return dbAtFreq;
}

const float MultiBandStep::rightDbAtFrequency (float frequency) const
{
    float dbAtFreq = 0;
    for (const auto& band : bands)
    {
        if (band.type == Band::Type::both || band.type == Band::Type::right)
        {
            dbAtFreq += dbAtFrequencyForBand (band, frequency);
        }
    }
    return dbAtFreq;
}

const float MultiBandStep::dbAtFrequencyForBand (Band band, float frequency) const
{
    float sampleRate = 44100; // just use this approximation, it should be fine, right?
    
    // Get parameters
    float f0 = band.freq;       // Center frequency
    float GdB = band.ampl;      // Gain in dB
    float Q = band.qFactor;     // Quality factor

    // Calculate normalized frequencies
    float omega0 = 2.0f * M_PI * f0 / sampleRate;
    float omega = 2.0f * M_PI * frequency / sampleRate;

    // Compute intermediate values
    float A = std::pow(10.0f, GdB / 40.0f);   // Amplitude (linear scale)
    float alpha = sinf(omega0) / (2.0f * Q);

    // Compute filter coefficients for peaking EQ
    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cosf(omega0);
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cosf(omega0);
    float a2 = 1.0f - alpha / A;

    // Compute the normalized frequency response
    float cos_omega = cosf(omega);
    float sin_omega = sinf(omega);
    float cos_2omega = cosf(2.0f * omega);
    float sin_2omega = sinf(2.0f * omega);

    // Numerator (real and imaginary parts)
    float num_real = b0 + b1 * cos_omega + b2 * cos_2omega;
    float num_imag = b1 * sin_omega + b2 * sin_2omega;

    // Denominator (real and imaginary parts)
    float den_real = a0 + a1 * cos_omega + a2 * cos_2omega;
    float den_imag = a1 * sin_omega + a2 * sin_2omega;

    // Compute magnitude squared of numerator and denominator
    float num_mag_sq = num_real * num_real + num_imag * num_imag;
    float den_mag_sq = den_real * den_real + den_imag * den_imag;

    // Compute the magnitude response (linear scale)
    float H_mag = sqrtf(num_mag_sq / den_mag_sq);

    // Convert to decibels
    float gainDB = 20.0f * log10f(H_mag);

    return gainDB;
}

//==============================================
BandProfile::BandProfile()
    : multiBandSteps ({}), volume (0.0f), melodyVolume (0.0f), noiseVolume (0.0f)
{}

BandProfile::BandProfile (std::vector<MultiBandStep> multiBandSteps, float volume, float melodyVolume, float noiseVolume)
    : multiBandSteps (multiBandSteps), volume (volume), melodyVolume (melodyVolume), noiseVolume (noiseVolume)
{}

const std::vector<MultiBandStep>& BandProfile::getMultiBandSteps() const
{
    return multiBandSteps;
}

std::vector<Band> BandProfile::getBands()
{
    std::vector<Band> bands;
    for (const auto& step : multiBandSteps)
    {
        bands.insert (bands.begin(), step.getBands().begin(), step.getBands().end()); // append all bands in each step
    }
    return bands;
}

const float BandProfile::getVolume() const
{
    return volume;
}

const float BandProfile::getMelodyVolume() const
{
    return melodyVolume;
}

const float BandProfile::getNoiseVolume() const
{
    return noiseVolume;
}
