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
#include "InverseFletcherMunsonCurve.h"

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
    }

    void setAmplitudes(std::vector<float> amplitudes)
    {
        this->amplitudes.clear();
        this->amplitudes.push_back (amplitudes[0]);
        for (int i = 1; i < amplitudes.size() - 1; ++i)
        {
            float avg;
            
            if (i == 1 || i == amplitudes.size() - 2)
            {
                avg = 0.5f * amplitudes[i - 1] + 0.5f * amplitudes[i + 1];
            }
            else if (i == 2 || i == amplitudes.size() - 3)
            {
                avg = 0.25f * amplitudes[i - 1] + 0.25f * amplitudes[i + 1] + 0.25f * amplitudes[i - 2] + 0.25f * amplitudes[i + 2];
            }
            else
            {
                float p = 1.0f / 6.0f;
                avg = p * (amplitudes[i - 3] + amplitudes[i - 2] + amplitudes[i - 1] + amplitudes[i + 1] + amplitudes[i + 2] + amplitudes[i + 3]);
            }
            
            if (abs (amplitudes[i] - avg) < 3.0f)
            {
                this->amplitudes.push_back (avg);
            }
            else
            {
                this->amplitudes.push_back (amplitudes[i]);
            }
        }
        this->amplitudes.push_back (amplitudes[amplitudes.size() - 1]);
    }

    void setPhases(std::vector<float> phases)
    {
        this->phases = phases;
    }

    void setPans(const std::vector<float>& pans)
    {
        this->pans = pans;
    }
    
    std::vector<float> frequencies;
    std::vector<float> amplitudes;

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float interpolatePhaseAtFrequency (const float frequency) const; // DRY violation
    
    
    std::vector<float> phases;
    std::vector<float> pans;
    float factor = 1.f;
    
    InverseFletcherMunsonCurve inverseFM;
};
