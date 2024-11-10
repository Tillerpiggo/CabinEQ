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

    // Returns frequency (Hz) and pan [-1, 1] at given time [0, 1)
    std::pair<float, float> getFrequencyAndPanAtTime(float time);

private:
    void generateCurve();
    uint32_t mortonEncode2D(uint32_t x, uint32_t y);
    void computeSegmentLengths();

    int complexity;
    int gridSize;
    int numPoints;

    std::vector<std::pair<float, float>> points;
    std::vector<float> segmentLengths;
    float totalLength;
};
