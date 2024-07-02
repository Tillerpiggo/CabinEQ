/*
  ==============================================================================

    Curve.h
    Created: 13 Jun 2024 8:25:42pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SetPointManager.h"

// This manages a curve interpolated between a list of set points, with an arbitrary resolution.
class Curve
{
public:
    Curve (SetPointManager& setPointManager)
    : frequencies (setPointManager.getSetPointFreqs()), amplitudes (setPointManager.getSetPointGains()) {}
    
    Curve (const std::vector<float>& frequencies, const std::vector<float>& amplitudes)
    : frequencies (frequencies), amplitudes (amplitudes) {}
    
    Curve (const std::vector<float>& frequencies, const std::vector<float>& amplitudes,
           const std::shared_ptr<std::vector<float>> phases)
    : frequencies (frequencies), amplitudes (amplitudes), phases (phases) {}
    
    Curve (const std::vector<float>& frequencies, const std::vector<float>& amplitudes, 
           const std::shared_ptr<std::vector<float>> phases, const std::shared_ptr<std::vector<float>> pans)
    : frequencies (frequencies), amplitudes (amplitudes), phases (phases), pans(pans) 
    {
        std::cout << "CURVE set point gains: ";
        for (float gain : amplitudes)
        {
            std::cout << gain << " ";
        }
        std::cout << std::endl;
    }
    
    virtual ~Curve() {}
    
    const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtNormalizedTime (float time) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const float cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const;
    
    void setFactor (const float factor)
    {
        this->factor = factor;
    }
    
protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const std::vector<float>& frequencies;
    const std::vector<float>& amplitudes;
    const std::optional<std::shared_ptr<std::vector<float>>> phases;
    const std::optional<std::shared_ptr<std::vector<float>>> pans;
    float factor = 1.f;
};
