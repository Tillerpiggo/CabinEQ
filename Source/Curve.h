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
            else if (i == 3 || i == amplitudes.size() - 4)
            {
                float p = 1.0f / 6.0f;
                avg = p * (amplitudes[i - 3] + amplitudes[i - 2] + amplitudes[i - 1] + amplitudes[i + 1] + amplitudes[i + 2] + amplitudes[i + 3]);
            }
            else if (i == 4 || i == amplitudes.size() - 5)
            {
                float p = 1.0f / 8.0f;
                avg = p * (amplitudes[i - 3] + amplitudes[i - 2] + amplitudes[i - 1] + amplitudes[i + 1] + amplitudes[i + 2] + amplitudes[i + 3] + amplitudes[i - 4] + amplitudes[i + 4]);
            }
            else if (i == 5 || i == amplitudes.size() - 6)
            {
                float p = 1.0f / 10.0f;
                avg = p * (amplitudes[i - 3] + amplitudes[i - 2] + amplitudes[i - 1] + amplitudes[i + 1] + amplitudes[i + 2] + amplitudes[i + 3] + amplitudes[i - 4] + amplitudes[i + 4] + amplitudes[i + 5] + amplitudes[i - 5]);
            }
            else
            {
                float p = 1.0f / 12.0f;
                avg = p * (amplitudes[i - 3] + amplitudes[i - 2] + amplitudes[i - 1] + amplitudes[i + 1] + amplitudes[i + 2] + amplitudes[i + 3] + amplitudes[i - 4] + amplitudes[i + 4] + amplitudes[i + 5] + amplitudes[i - 5] + amplitudes[i + 6] + amplitudes[i - 6]);
            }
            
            if (abs (amplitudes[i] - avg) < 3.5f)
            {
                this->amplitudes.push_back (avg);
            }
            else
            {
                float avgDiff = abs (amplitudes[i] - avg);
                float diffThreshold = 4.0f; // Amount of difference from average to use purely the original
                if (avgDiff > diffThreshold) avgDiff = diffThreshold;
                float percentOrig = (diffThreshold - abs (amplitudes[i] - avg)) / diffThreshold;
                percentOrig *= percentOrig;
                percentOrig *= percentOrig;
                this->amplitudes.push_back (percentOrig * amplitudes[i] + (1 - percentOrig) * avg);
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
        
        this->pans.push_back (pans[0]);
        for (int i = 1; i < pans.size() - 1; ++i)
        {
            float avg;
            
            if (i == 1 || i == pans.size() - 2)
            {
                avg = 0.5f * pans[i - 1] + 0.5f * pans[i + 1];
            }
            else if (i == 2 || i == pans.size() - 3)
            {
                avg = 0.25f * pans[i - 1] + 0.25f * pans[i + 1] + 0.25f * pans[i - 2] + 0.25f * pans[i + 2];
            }
            else
            {
                float p = 1.0f / 6.0f;
                avg = p * (pans[i - 3] + pans[i - 2] + pans[i - 1] + pans[i + 1] + pans[i + 2] + pans[i + 3]);
            }
            
            if (abs (pans[i] - avg) < 1.5f)
            {
                this->pans.push_back (avg);
            }
            else
            {
                float avgDiff = abs (pans[i] - avg);
                float diffThreshold = 2.0f; // Amount of difference from average to use purely the original
                if (avgDiff > diffThreshold) avgDiff = diffThreshold;
                float percentOrig = (diffThreshold - abs (pans[i] - avg)) / diffThreshold;
                this->pans.push_back (percentOrig * pans[i] + (1 - percentOrig) * avg);
            }
        }
        this->pans.push_back (pans[pans.size() - 1]);
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
