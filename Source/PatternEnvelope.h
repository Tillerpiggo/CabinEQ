/*
  ==============================================================================

    PatternEnvelope.h
    Created: 9 Feb 2025 5:57:13pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class modulates the gain of a noise source according to a bank of predefined patterns
class PatternEnvelope
{
public:
    PatternEnvelope() {};
    
    float volumeAtTime (float time, int patternIdx) const;
    
private:
    std::vector<std::vector<int>> patterns = {{ 1, 1, 0, 0, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1, 0, 1, 0 }, { 1, 0, 0, 1, 0, 0, 1, 0 }};
    int tempo = 8;
};
