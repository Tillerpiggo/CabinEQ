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

#include "Constants.h"
#include "CurvePt.h"

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const float compensatedValueAtFrequency (float frequency, float compensationSlope) const;
    const float valueAtFrequency (float frequency);
    virtual const float valueAtTime (float time);
    
    float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
    const std::vector<CurvePt>& getCurvePts();
    void updateWithCurvePts (std::vector<CurvePt> curvePts);
    
//    const float* getImpulse (int fft_size); // This hands ownership of the float*'s to whoever calls it!!

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float visualInterpolateAmplitudeAtFrequency (const float frequency) const; // for display on graph
//    float* frequencyResponse (int numPoints);
    
    std::vector<CurvePt> curvePts;
    
    std::unordered_map<float, float> cache;
};

class FlatCurve   : public Curve
{
public:
    using Curve::Curve;
    const float valueAtTime (float time) override
    {
        return 1.0f;
    }
};
