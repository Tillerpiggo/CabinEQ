/*
  ==============================================================================

    SpatialPatternGenerator.cpp
    Created: 12 Sep 2024 11:53:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SpatialPatternGenerator.h"

SpatialPatternGenerator::SpatialPatternGenerator()
    : numSamplesNoteHasBeenPlaying (0)
{
}

void SpatialPatternGenerator::setSampleRate (float sampleRate)
{
    noiseGenerator.setSampleRate (sampleRate);
}

void SpatialPatternGenerator::setAmplCurve (Curve& amplCurve)
{
    noiseGenerator.setAmplCurve (amplCurve);
}

void SpatialPatternGenerator::setPattern (std::vector<NoiseNote> notes)
{
    this->notes = notes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    updateBandpassAndPanning();
}

void SpatialPatternGenerator::setCenterFrequency (float centerFrequency)
{
    this->centerFrequency = centerFrequency;
}

std::pair<float, float> SpatialPatternGenerator::getNextSample()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return { 0, 0 };
    
    std::pair<float, float> sample = noiseGenerator.getNextSample();
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples)
    {
        goToNextNote();
    }

    // Apply panning
    return { sample.first * leftGain, sample.second * rightGain };
}

NoiseNote SpatialPatternGenerator::getCurrNote()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return notes[0];
    return notes[currNoteIdx];
}

void SpatialPatternGenerator::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
    {
        currNoteIdx = 0;
    }
    
    updateBandpassAndPanning();
}

void SpatialPatternGenerator::updateBandpassAndPanning()
{
    float bandpassFrequency = getCurrNote().freqFactor * centerFrequency;
    
    // Adjust leftgain and rightgain according to the curr note's angle
    float angle = getCurrNote().pan * M_PI / 4.0f; // go from [-1, 1] to [-pi/4, pi/4]
    leftGain = std::sqrt (2.0f) / 2.0f * (std::cos (angle) - std::sin(angle));
    rightGain = std::sqrt (2.0f) / 2.0f * (std::cos(angle) + std::sin(angle));
    noiseGenerator.setBandpass (bandpassFrequency, getCurrNote().bandwidth);
}
