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
        
        std::cout << "Frequencies: " << std::endl;
        for (float freq : frequencies) std::cout << freq << " ";
        std::cout << std::endl;
    }

    void setAmplitudes(std::vector<float> amplitudes)
    {
        this->amplitudes = amplitudes;
        std::cout << "Amplitudes: " << std::endl;
        for (float amp : amplitudes) std::cout << amp << " ";
        std::cout << std::endl;
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
                float diffThreshold = 1.5f; // Amount of difference from average to use purely the original
                if (avgDiff > diffThreshold) avgDiff = diffThreshold;
                float percentOrig = (diffThreshold - abs (pans[i] - avg)) / diffThreshold;
                percentOrig *= percentOrig;
                percentOrig *= percentOrig;
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
    float factor = 1.0f;
};
