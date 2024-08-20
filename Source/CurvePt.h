/*
  ==============================================================================

    CurvePoint.h
    Created: 19 Aug 2024 11:53:44am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

/// This stores a single (freq, val) pair that is used to define a Curve and savable in the profile
struct CurvePt
{
    CurvePt (int id, float freq, float val)
        : id (id), freq (freq), val (val)
    {}
    
    int id;
    float freq;
    float val;
};
