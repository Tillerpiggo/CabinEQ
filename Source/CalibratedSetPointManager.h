/*
  ==============================================================================

    CalibratedSetPointManager.h
    Created: 1 Jul 2024 3:34:15pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include "Curve.h"

class CalibratedSetPointManager
{
public:
    CalibratedSetPointManager();
    
    
    const Curve& getCurve() { return curve; }
    
private:
    Curve curve;
    
    
};

