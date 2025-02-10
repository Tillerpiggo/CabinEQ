/*
  ==============================================================================

    Checkerboard.cpp
    Created: 10 Feb 2025 3:19:14pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "Checkerboard.h"

Checkerboard::Checkerboard()
    : resolution (2), polarity (true)
{
    
}

Checkerboard::Checkerboard (int resolution, bool polarity)
    : resolution (resolution), polarity (polarity)
{
}

int Checkerboard::getResolution()
{
    return resolution;
}

bool Checkerboard::getPolarity()
{
    return polarity;
}


void Checkerboard::togglePolarity()
{
    polarity = ! polarity;
}
