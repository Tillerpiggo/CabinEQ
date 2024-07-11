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
    std::vector<float> amplitudePoints = sliderSetPointManager.getAmplitudes();
    std::vector<float> frequencies = sliderSetPointManager.getFrequencies();
    
    for (int i = 0; i < amplitudePoints.size(); ++i)
    {
        std::cout << "Slider Amplitude at " << frequencies.at (i) << ": " << amplitudePoints.at (i);
    }
    
    curve.setAmplitudes (amplitudePoints);
    std::cout << "set amplitudes" << std::endl;
    curve.setPans (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
    std::cout << "set pans" << std::endl;
    curve.setPhases (std::vector<float> (sliderSetPointManager.numPoints(), 0.0f));
    std::cout << "set phases" << std::endl;
    
    filter.update (curve, 18);
    std::cout << "updated filter" << std::endl;
}
