///*
//  ==============================================================================
//
//    Curve.h
//    Created: 13 Jun 2024 8:25:42pm
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#pragma once
//
//#include <JuceHeader.h>
//#include <vector>
//#include <complex>
//#include <cmath>
//
//class Curve
//{
//public:
//    Curve() = default;
//    virtual ~Curve() = default;
//    
//    virtual const float valueAtTime (float time); // returns value in dB
//    
//    std::vector<float> getFrequencyResponse (int numPoints); // Returns frequency response in a float* representing gain for each freq at each point w/ len 2 * numPoints
//
//    static std::pair<std::vector<float>, std::vector<float>> getStereoFrequencyResponse (Curve& amplCurve, int numPoints); // returns stereo frequency response, with real values interweaved with imaginary values for each complex number
////    const float* getImpulse (int fft_size); // This hands ownership of the float*'s to whoever calls it!!
//
//protected:
//};
//
//class FlatCurve   : public Curve
//{
//public:
//    using Curve::Curve;
//    const float valueAtTime (float time) override
//    {
//        return 1.0f;
//    }
//};


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

#include "CurvePt.h"

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const std::pair<float, float> valueAtFrequency (float frequency) const;
    virtual const std::pair<float, float> valueAtTime (float time);
    
    float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
    void updateWithCurvePts (std::vector<CurvePt> leftCurvePts, std::vector<CurvePt> rightCurvePts);
    static std::pair<std::vector<float>, std::vector<float>> getStereoFrequencyResponse (Curve& amplCurve, int numPoints);

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<CurvePt>& curvePts) const;
    
    std::vector<CurvePt> leftCurvePts;
    std::vector<CurvePt> rightCurvePts;
};

class TiltCurve  : public Curve
{
public:
    const std::pair<float, float> valueAtTime (float time) override
    {
        float freq = std::max (20.0f, time * 22050.0f);
        float val = -3.0f * std::log2 (freq / 1000.0f);
        return { val, val };
    }
};
