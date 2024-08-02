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

#include "EQNode.h"

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const std::pair<std::complex<float>, std::complex<float>> compensatedValueAtFrequency (float frequency, float compensationSlope) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtNormalizedTime (float time) const;
    float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
    void updateWithEQNodes (std::vector<EQNode> eqNodes);
    
    const std::pair<float*, float*> getStereoImpulse (int fft_size) const; // This hands ownership of the float*'s to whoever calls it!!

protected:
    const std::pair<std::complex<float>, std::complex<float>> addComplexPair (std::pair<std::complex<float>, std::complex<float>> pair1, std::pair<std::complex<float>, std::complex<float>> pair2) const;
    const std::pair<std::complex<float>, std::complex<float>> multiplyComplexPair (std::pair<std::complex<float>, std::complex<float>> pair1, std::pair<std::complex<float>, std::complex<float>> pair2) const;
    const std::pair<std::complex<float>, std::complex<float>> scaleComplexPair (std::pair<std::complex<float>, std::complex<float>> pair, float scalar) const;
    const std::pair<std::complex<float>, std::complex<float>> reciprocalComplexPair (std::pair<std::complex<float>, std::complex<float>> pair) const;
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    std::pair<float*, float*> frequencyResponse (int numPoints) const;
    
    std::vector<EQNode> eqNodes;
};
