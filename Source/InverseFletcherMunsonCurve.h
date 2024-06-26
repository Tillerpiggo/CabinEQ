/*
  ==============================================================================

    InverseFletcherMunsonCurve.h
    Created: 25 Jun 2024 12:18:19pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class InverseFletcherMunsonCurve
{
public:
    InverseFletcherMunsonCurve () {}
    
    const std::complex<float> valueAtFrequency (float frequency) const;
    const float catmullRom (float t, float y0, float y1, float y2, float y3) const;
    const std::vector<float> iso226(float L_n, std::vector<float> &frequencies) const;
    
};
