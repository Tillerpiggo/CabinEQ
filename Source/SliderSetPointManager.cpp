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
    frequencies = { 20, 40, 80, 160, 240, 320, 640, 1280, 1800, 2560, 3500, 4000, 5120, 6000, 7000, 8000, 9000, 10240, 11000, 12000, 13000, 14000, 15000 };
    
    for (int i = 0; i < frequencies.size(); ++i)
    {
        amplitudes.push_back (0.0f);
    }
}
