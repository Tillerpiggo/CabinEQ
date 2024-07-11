/*
  ==============================================================================

    SliderSetPointManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SliderSetPointManager
{
public:
    SliderSetPointManager();
    
    const std::vector<float>& getFrequencies() const { return frequencies; }
    const std::vector<float>& getAmplitudes() const { return amplitudes; }
    const int numPoints() const { return frequencies.size(); }
    
    float getFrequencyAt (int index) const 
    {
        return frequencies.at (index);
    }
    
    float getAmplitudeAt (int index) const 
    {
        return amplitudes.at (index);
    }
    
    void setAmplitudeAt (int index, float newAmplitude) 
    {
        amplitudes.at (index) = newAmplitude;
    }
    
private:
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
};
