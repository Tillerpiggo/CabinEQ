/*
  ==============================================================================

    SliderSetPointManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSetPointManager.h"

SliderSetPointManager::SliderSetPointManager()
{
    // All C, E, G, A for octaves in human hearing range
//    std::vector<float> baseFrequencies = { 32.7032, 41.2034, 48.9994, 55.0000 }; // C1, E1, G1, A1
//
//    // Initial frequencies at base frequencies and half of them
//    for (int octave = -1; octave < 9; ++octave)
//    {
//        float scaleFactor = std::pow (2.0, octave);
//        for (float baseFreq : baseFrequencies)
//        {
//            frequencies.push_back(baseFreq * scaleFactor);
//        }
//    }
    
//    frequencies = { 20, 40, 80, 160, 240, 320, 640, 1280, 1800, 2560, 3500, 4000, 5120, 6000, 7000, 8000, 9000, 10240, 11000, 12000, 13000, 14000, 15000 };
//    frequencies = { 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240 };
//    frequencies = { 20, 40, 80, 160, 320, 640, 1000, 2000, 3000, 4000, 5000, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 10500, 11000, 11500, 12000 };
    
    //frequencies = { 20, 30, 40, 54, 80, 100, 160, 190, 240, 320, 440, 640, 800, 1280, 1400, 1800, 2000, 2560, 3000, 3500, 4000, 5120, 6000, 7000, 8000, 9000, 10240 };
    frequencies = { 20, 40, 80, 160, 200, 320, 403, 640, 1280, 1612, 2032, 2560, 2940, 3377, 3880, 4457, 5120, 5881, 6755, 7760, 8914, 10240, 14000 };
    
    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back(0.0f);
        pans.push_back(0.0f);
    }
}
