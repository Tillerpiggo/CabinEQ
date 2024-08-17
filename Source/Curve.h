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
#include "EQNode.h"

class Curve
{
public:
    Curve() = default;
    virtual ~Curve() = default;

    const std::pair<std::complex<float>, std::complex<float>> compensatedValueAtFrequency (float frequency, float compensationSlope) const;
    const std::pair<std::complex<float>, std::complex<float>> avgValueAtFrequency (float frequency, float compensationSlope) const;
    const std::pair<std::complex<float>, std::complex<float>> utilValueAtFrequency (float frequency, float compensationSlope) const;
    const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency);
    const std::pair<std::complex<float>, std::complex<float>> valueAtTime (float time);
    
    float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
    const std::vector<EQNode>& getEQNodes();
    void updateWithEQNodes (std::vector<EQNode> eqNodes);
    
    const std::pair<float*, float*> getStereoImpulse (int fft_size); // This hands ownership of the float*'s to whoever calls it!!
    
    const std::optional<std::pair<float, float>> nodeBelowFreq (float frequency);
    const std::optional<std::pair<float, float>> nodeAboveFreq (float frequency);

protected:
    const std::pair<std::complex<float>, std::complex<float>> scaleComplexPair (std::pair<std::complex<float>, std::complex<float>> pair, float scalar) const;
    const float interpolateValueAtFrequency (const float frequency, const std::vector<float>& values) const;
    const float visualInterpolateAmplitudeAtFrequency (const float frequency) const; // for display on graph
    std::pair<float*, float*> frequencyResponse (int numPoints);
    
    std::vector<EQNode> eqNodes;
    
    std::unordered_map<float, std::pair<std::complex<float>, std::complex<float>>> cache;
};
