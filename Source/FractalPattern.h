/*
  ==============================================================================

    FractalPattern.h
    Created: 9 Nov 2024 11:04:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// This class represents a space-filling curve pattern with a certain complexity that fills a space. It uses this to give you the frequency and panning at any time along the curve.
class FractalPattern
{
public:
    FractalPattern(int complexity);

    std::pair<float, float> getFrequencyAndPanAtTime(float time); // time is from [0, 1)

private:
    void generateMooreCurve();
    void mooreCurve(int level, int dir, int &x, int &y);
    void rot(int n, int &x, int &y, int rx, int ry);

    int complexity;
    int order;
    int size;
    int numPoints;

    std::vector<std::pair<float, float>> points;
};
