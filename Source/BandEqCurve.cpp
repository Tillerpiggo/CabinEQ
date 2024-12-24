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

//const float BandEqCurve::dbAtFrequencyForBand (Band band, float frequency) const
//{
//    // Compute frequency difference in octaves
//    float x = std::log2(frequency / band.freq);
//    
//    // Calculate exponent for the exponential function
//    float ln2 = std::log(2.0f);
//    float exponent = -ln2 * (4.0f * x * x) / (band.bandwidth * band.bandwidth);
//    
//    // Compute gain in dB directly
//    float gainDB = band.ampl * std::exp(exponent);
//    
//    return gainDB;
//}

const float BandEqCurve::dbAtFrequencyForBand (Band band, float frequency) const
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
