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
    
    virtual const float valueAtTime (float time); // returns value in dB
    
    std::vector<float> getFrequencyResponse (int numPoints); // Returns frequency response in a float* representing gain for each freq at each point w/ len 2 * numPoints

    static std::pair<std::vector<float>, std::vector<float>> getStereoFrequencyResponse (Curve& amplCurve, int numPoints); // returns stereo frequency response, with real values interweaved with imaginary values for each complex number
//    const float* getImpulse (int fft_size); // This hands ownership of the float*'s to whoever calls it!!

protected:
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
