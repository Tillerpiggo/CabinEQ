/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

SineWaveGenerator::SineWaveGenerator ()
{
   // currentGain.reset (rampLengthInSamples);
}

void SineWaveGenerator::setSampleRate (float newSampleRate)
{
    sampleRate = newSampleRate;
}

const std::pair<float, float> SineWaveGenerator::getNextSample()
{
    float leftSample = std::sin (phase) * leftAmplitudeCompensation;
    float rightSample = std::sin (phase + note->phase) * rightAmplitudeCompensation;
    
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<float>::pi)
        phase -= 2.0 * juce::MathConstants<float>::pi;
    
    return { leftSample, rightSample };
}

// Sets the note to the newNote, and also "plays" it by ending the last note
// and starting the new note with a gain ramp
void SineWaveGenerator::setNote (Note newNote)
{
    note = newNote;
    phase = 0;
    updatePhaseIncrementAndAmplitudeCompensation();
}

void SineWaveGenerator::setVolume (float gainInDecibels)
{
    note->gain = gainInDecibels;
    updatePhaseIncrementAndAmplitudeCompensation();
}

void SineWaveGenerator::setPan (float panInDecibels)
{
    // TODO: create gain ramp
    note->pan = panInDecibels;
    updatePhaseIncrementAndAmplitudeCompensation();
}

void SineWaveGenerator::setPhase (float phaseInRadians)
{
    note->phase = phase;
    updatePhaseIncrementAndAmplitudeCompensation();
}

// ============================================
void SineWaveGenerator::updatePhaseIncrementAndAmplitudeCompensation()
{
    if (! note)
    {
        throw std::runtime_error("updatePhaseIncrementAndAmplitudeCompensation() called in SineWaveGenerator before setting the note to be played");
    }
    
    phaseIncrement = 2.0 * juce::MathConstants<float>::pi * note->frequency / sampleRate;
    float amplitudeCompensation = std::pow (TILT, std::log2(note->frequency / REFERENCE_FREQ));
    float noteGain = juce::Decibels::decibelsToGain (note->gain + 30.0f);
    amplitudeCompensation *= noteGain;
    
    leftAmplitudeCompensation = amplitudeCompensation;
    rightAmplitudeCompensation = amplitudeCompensation;
    
    // Apply panning
    leftAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / -2.0);
    rightAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / 2.0);
}
