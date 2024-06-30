/*
  ==============================================================================

    BalanceCurve.h
    Created: 30 Jun 2024 10:34:03am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "SetPointManager.h"

class BalanceCurve  : public Curve
{
public:
    BalanceCurve (SetPointManager& panSetPointManager, SetPointManager& phaseSetPointManager) : Curve (panSetPointManager), phaseSetPointManager (phaseSetPointManager) {}
    
    virtual const std::pair<std::complex<float>, std::complex<float>> valueAtFrequency (float frequency) const override
    {
        float panAtFrequency = valueAtFrequencyForSetPointManager (frequency, setPointManager);
        float phaseAtFrequency = valueAtFrequencyForSetPointManager (frequency, phaseSetPointManager);
        
        float leftGain = juce::Decibels::decibelsToGain (-0.5 * panAtFrequency);
        float rightGain = juce::Decibels::decibelsToGain (0.5 * panAtFrequency);
        
        std::complex<float> leftVal = std::polar (leftGain, 0.0f);
        std::complex<float> rightVal = std::polar (rightGain, phaseAtFrequency);
        
        return { leftVal, rightVal };
    }
    
private:
    SetPointManager& phaseSetPointManager;
};
