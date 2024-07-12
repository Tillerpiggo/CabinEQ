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
    updateCurve();
    filter.update (curve, 21);
}

void SliderCalibrationManager::updateCurve()
{
    curve.setFrequencies (sliderSetPointManager.getFrequencies());
    std::vector<float> amplitudePoints = sliderSetPointManager.getAmplitudes();
    std::vector<float> panPoints = sliderSetPointManager.getPans();
    std::vector<float> frequencies = sliderSetPointManager.getFrequencies();
    
    curve.setAmplitudes (amplitudePoints);
    curve.setPans (panPoints);
    curve.setPhases (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
}
