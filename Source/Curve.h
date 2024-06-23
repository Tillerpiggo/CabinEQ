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
#include "CalibratedSetPoint.h"

// This manages a curve interpolated between a list of set points, with an arbitrary resolution.
class Curve
{
public:
    Curve () : setPointMap () {}
    Curve (std::shared_ptr<std::map<float, CalibratedSetPoint>> setPointMap) : setPointMap (setPointMap) {}
    virtual ~Curve() {}
    
    virtual const std::complex<float> valueAtFrequency (float frequency) const;
    const std::complex<float> valueAtTime (float time) const;
    const std::complex<float> valueAtNormalizedTime (float time) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const float cubicBezierWithHorizontalDerivative (float t, float y0, float y1) const;
    
    void setSetPointMap (std::shared_ptr<std::map<float, CalibratedSetPoint>> setPointMap)
    {
        this->setPointMap = setPointMap;
    }
    void setFactor (const float factor)
    {
        this->factor = factor;
    }
    
protected:
    std::shared_ptr<std::map<float, CalibratedSetPoint>> setPointMap;
    float factor = 1.f;
};

class FlatCurve : public Curve 
{
public:
    using Curve::Curve;

    // Return a flat curve
    const std::complex<float> valueAtFrequency(float frequency) const override {
        return std::complex<float>(0.5, 0);
    }
};

class LinearCurve : public Curve
{
public:
    using Curve::Curve;
    const std::complex<float> valueAtFrequency (float time) const;
};
