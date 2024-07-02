/*
  ==============================================================================

    CalibratedSetPointManager.h
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Curve.h"
#include "CalibratedSetPoint.h"

class CalibratedSetPointManager
{
public:
    CalibratedSetPointManager();
    
    const Curve& getCurve() const { return curve; }
    
private:
    std::vector<float> frequencies;
    std::vector<CalibratedSetPoint> amplitudes;
    std::vector<CalibratedSetPoint> pans;
    std::vector<CalibratedSetPoint> phases;
    
    Curve curve;
};

