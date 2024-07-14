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
    frequencies = { 20, 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240 };
    
    
    for (const auto& freq : frequencies) std::cout << "freq: " << freq << std::endl;
    
    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back(0.0f);
        pans.push_back(0.0f);
    }
}
