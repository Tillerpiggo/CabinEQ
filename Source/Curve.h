/*
  ==============================================================================

    Curve.h
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtNormalizedTime (float time) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const float cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const;
    
    void setFactor (const float factor)
    {
        this->factor = factor;
    }

    void setFrequencies(std::vector<float> frequencies)
    {
        this->frequencies = frequencies;
        this->frequencies.push_back (22050);
    }

    void setAmplitudes(std::vector<float> amplitudes)
    {
        this->amplitudes = amplitudes;
        float lastAmplitude = amplitudes.at (amplitudes.size() - 1);
        this->amplitudes.push_back (lastAmplitude);
    }

    void setPhases(std::vector<float> phases)
    {
        this->phases = phases;
        this->phases.push_back (0);
    }

    void setPans(const std::vector<float>& pans)
    {
        this->pans = pans;
        this->pans.push_back (0.0f);
    }

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float interpolatePhaseAtFrequency (const float frequency) const; // DRY violation
    
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    std::vector<float> phases;
    std::vector<float> pans;
    float factor = 1.f;
};
