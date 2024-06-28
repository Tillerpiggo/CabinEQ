/*
  ==============================================================================

    HarmanCurve.h
    Created: 27 Jun 2024 3:43:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <complex>
#include <cmath>

class HarmanCurve
{
public:
    HarmanCurve () {}
    
    const std::complex<float> valueAtFrequency (float frequency) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    
private:
    const std::vector<float> frequencies = {
        10.000, 20.000, 30.000, 40.000, 50.000, 60.000, 70.000, 80.000,
        90.000, 100.000, 110.000, 120.000, 140.000, 160.000, 180.000,
        200.000, 220.000, 240.000, 300.000, 400.000, 20000.000
    };
    
    const std::vector<float> gains = {
        4.199, 4.184, 4.119, 3.949, 3.616, 3.082, 2.356, 1.489,
        0.560, -0.361, -1.219, -1.984, -3.203, -4.043, -4.601,
        -4.968, -5.211, -5.374, -5.619, -5.742, -5.742
    };
};
