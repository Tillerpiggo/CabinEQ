/*
  ==============================================================================

    SliderSetPointManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SliderSetPointManager.h"

//SliderSetPointManager::SliderSetPointManager()
//{
//    // All C, E, G, A for octaves in human hearing range
//    std::vector<float> baseFrequencies = { 32.7032, 41.2034, 48.9994, 55.0000 }; // C1, E1, G1, A1
//
//    for (float baseFreq : baseFrequencies)
//    {
//        frequencies.push_back(baseFreq / 2.0);
//        frequencies.push_back(baseFreq);
//    }
//    
//    size_t currentSize = frequencies.size();
//    while (frequencies.back() < 20000.0)
//    {
//        for (size_t i = 0; i < currentSize; ++i)
//        {
//            float newFreq = frequencies[i] * 2.0;
//            if (newFreq < 20000.0)
//            {
//                frequencies.push_back(newFreq);
//            }
//        }
//        currentSize = frequencies.size();
//    }
//    
//    for (int i = 0; i < frequencies.size(); ++i)
//    {
//        amplitudes.push_back(0.0f);
//        pans.push_back(0.0f);
//    }
//}

/*
SliderSetPointManager::SliderSetPointManager()
{
    // All C, E, G, A for octaves in human hearing range
    std::vector<float> baseFrequencies = { 32.7032, 41.2034, 48.9994, 55.0000 }; // C1, E1, G1, A1

    for (float baseFreq : baseFrequencies)
    {
        frequencies.push_back(baseFreq / 2.0);
        frequencies.push_back(baseFreq);
    }
    
    size_t currentSize = frequencies.size();
    size_t index = 0;

    while (true)
    {
        float newFreq = frequencies[index] * 2.0;
        if (newFreq >= 20000.0)
        {
            break;
        }
        frequencies.push_back(newFreq);
        ++index;
        
        // Update the size to reflect the newly added frequencies
        if (index == currentSize)
        {
            currentSize = frequencies.size();
        }
    }

    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back(0.0f);
        pans.push_back(0.0f);
    }
}
*/

SliderSetPointManager::SliderSetPointManager()
{
    // All C, E, G, A frequencies from below 20hz to 20khz
    std::vector<float> baseFrequencies = { 32.7032, 41.2034, 48.9994, 55.0000 }; // C1, E1, G1, A1
    std::vector<float> extendedFrequencies;
    for (float baseFreq : baseFrequencies)
    {
        extendedFrequencies.push_back(baseFreq / 2.0);
    }
    extendedFrequencies.insert(extendedFrequencies.end(), baseFrequencies.begin(), baseFrequencies.end());
    
    while (extendedFrequencies.back() < 20000.0)
    {
        for (float freq : extendedFrequencies) frequencies.push_back(freq);
        for (float& freq : extendedFrequencies) freq *= 2.0;
    }
    
    for (int i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back(0.0f);
        pans.push_back(0.0f);
    }
}
