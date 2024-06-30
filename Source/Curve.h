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
    Curve (SetPointManager& setPointManager) : setPointManager (setPointManager) {}
    virtual ~Curve() {}
    
    virtual const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtNormalizedTime (float time) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const float cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const;
    
    void setFactor (const float factor)
    {
        this->factor = factor;
    }
    
protected:
    const float valueAtFrequencyForSetPointManager (const float frequency, const SetPointManager& setPointManager) const;
    const SetPointManager& setPointManager;
    float factor = 1.f;
};
