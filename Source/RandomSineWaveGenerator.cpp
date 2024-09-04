/*
  ==============================================================================

    RandomSineWaveGenerator.cpp
    Created: 27 Aug 2024 11:12:45pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "RandomSineWaveGenerator.h"
#include <iostream>
#include <random>
#include <cmath>

RandomSineWaveGenerator::RandomSineWaveGenerator()
{
    sineWaveGenerator.setNote (currNote.getNote());
}

void RandomSineWaveGenerator::setSampleRate (float newSampleRate)
{
    sineWaveGenerator.setSampleRate (newSampleRate);
}

void RandomSineWaveGenerator::setCurve (Curve curve)
{
    this->curve = curve;
}

const std::pair<float, float> RandomSineWaveGenerator::getNextSample()
{
    if (samplesNoteHasBeenPlaying > noteLengthInSamples)
    {
        randomizeNote();
        samplesNoteHasBeenPlaying = 0;
    }
    
    auto [leftSample, rightSample] = sineWaveGenerator.getNextSample();
    auto [leftGain, rightGain] = currNote.getGainAtSample (samplesNoteHasBeenPlaying);
//    
    samplesNoteHasBeenPlaying++;
    
    return { leftSample * leftGain, rightSample * rightGain };
}

void RandomSineWaveGenerator::randomizeNote()
{
    // Generate random frequency from 20 to 20000hz, logarithmically
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(std::log(40.0), std::log(20000.0));
    float logFrequency = dis(gen);
    float frequency = std::exp(logFrequency);
    
    float ampl = -8.0 * std::log2 (frequency / 1000.0f); // musical slant
    if (curve.has_value())
        ampl += curve->valueAtFrequency (frequency);
    
    currNote = SequenceableNote (frequency, ampl, 0.0f, 0.0f, noteLengthInSamples, StereoGainEnvelope (500));
    sineWaveGenerator.setNote (currNote.getNote());
}
