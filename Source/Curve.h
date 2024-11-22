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

//#include "Constants.h"
#include "CurvePt.h"

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const float valueAtFrequency (float frequency) const; // returns value in dB
    const float visualValueAtFrequency (float frequency); // returns value in dB
    virtual const float valueAtTime (float time); // returns value in dB
    
    float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
    const std::vector<CurvePt>& getCurvePts();
    void updateWithCurvePts (std::vector<CurvePt> curvePts);
    
    std::vector<float> getFrequencyResponse (int numPoints); // Returns frequency response in a float* representing gain for each freq at each point w/ len 2 * numPoints
    
    const std::optional<std::pair<float, float>> nodeBelowFreq (float frequency);
    const std::optional<std::pair<float, float>> nodeAboveFreq (float frequency);
    const std::optional<std::vector<float>> getFirstThreeFreqs();
    const std::optional<std::vector<float>> getFirstFourFreqs();

    static std::pair<std::vector<float>, std::vector<float>> getStereoFrequencyResponse (Curve& amplCurve, /*Curve& panCurve, Curve& phaseCurve,*/ int numPoints); // returns stereo frequency response, with real values interweaved with imaginary values for each complex number
//    const float* getImpulse (int fft_size); // This hands ownership of the float*'s to whoever calls it!!

protected:
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float visualInterpolateAmplitudeAtFrequency (const float frequency) const; // for display on graph
    
    
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


