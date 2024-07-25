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
#include <cmath>

#include "SetPoint.h"

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
    
    void setFactor (const float factor);
    void setFrequencies(std::vector<float> frequencies);
    void setAmplitudes(std::vector<float> amplitudes);
    void setPhases(std::vector<float> phases);
    void setPans(const std::vector<float>& pans);
    
    void updateWithSetPoints (std::vector<SetPoint> setPoints);
    
    const std::pair<float*, float*> getStereoImpulse (int fft_size) const; // This hands ownership of the float*'s to whoever calls it!!
    
    std::vector<float> frequencies;
    std::vector<float> amplitudes;

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float interpolatePhaseAtFrequency (const float frequency) const; // DRY violation
    std::pair<float*, float*> frequencyResponse (int numPoints) const;
    
    std::vector<float> phases;
    std::vector<float> pans;
    float factor = 1.0f;
};
