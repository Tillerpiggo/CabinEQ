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
    const std::vector<float>& getPans() const { return pans; }
    const int numPoints() const { return static_cast<int> (frequencies.size()); }
    
    float getFrequencyAt (int index) const 
    {
        return frequencies.at (index);
    }
    
    float getAmplitudeAt (int index) const 
    {
        return amplitudes.at (index);
    }
    
    float getPanAt (int index) const
    {
        return pans.at (index);
    }
    
    int getNumPoints() const
    {
        return frequencies.size();
    }
    
    void setAmplitudeAt (int index, float newAmplitude) 
    {
        amplitudes.at (index) = newAmplitude;
    }
    
    void setPanAt (int index, float newPan)
    {
        pans.at (index) = newPan;
    }
    
    int indexForFrequency (float frequency) const
    {
        for (int i = 0; i < frequencies.size(); ++i)
        {
            if (frequency == frequencies.at (i))
            {
                return i;
            }
        }
        
        return -1;
    }
    
    static const int NUM_PTS = 22;
    
private:
    std::vector<float> frequencies;
    std::vector<float> amplitudes;
    std::vector<float> pans;
};
