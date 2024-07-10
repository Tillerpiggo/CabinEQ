/*
  ==============================================================================

    SliderCalibrationManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderCalibrationManager.h"

void SliderCalibrationManager::updateFilter (ArbitraryResponseFilter& filter)
{
    curve.setFrequencies (sliderSetPointManager.getFrequencies());
    curve.setAmplitudes (sliderSetPointManager.getAmplitudes());
    curve.setPans (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
    curve.setPhases (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
    
    filter.update (curve, 18);
}
