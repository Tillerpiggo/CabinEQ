/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

SineWaveGenerator::SineWaveGenerator (bool applyCompensation) : applyCompensation (applyCompensation)
{
   // currentGain.reset (rampLengthInSamples);
}

void SineWaveGenerator::setSampleRate (float newSampleRate)
{
    sampleRate = newSampleRate;
}

const std::pair<float, float> SineWaveGenerator::getNextSample()
{
    if (targetFrequency.has_value())
    {
        if (targetFrequency.value() > note->frequency)
        {
            note->frequency *= FREQ_STEP;
        }
        else
        {
            note->frequency /= FREQ_STEP;
        }
        
        float ratio = note->frequency / targetFrequency.value();
        if (ratio < FREQ_STEP && ratio > 1.0f / FREQ_STEP)
        {
            targetFrequency.reset();
        }
        updatePhaseIncrementAndAmplitudeCompensation();
    }
    
    if (targetAmplitude.has_value())
    {
        if (targetAmplitude.value() > note->gain)
        {
            note->gain *= AMPL_STEP;
        }
        else
        {
            note->gain /= AMPL_STEP;
        }
        
        float ratio = note->gain / targetAmplitude.value();
        if (ratio < AMPL_STEP && ratio > 1.0f / AMPL_STEP)
        {
            targetAmplitude.reset();
        }
        updatePhaseIncrementAndAmplitudeCompensation();
    }
    
    
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
    updatePhaseIncrementAndAmplitudeCompensation();
    targetFrequency.reset();
    targetAmplitude.reset();
}

void SineWaveGenerator::setFrequency (float frequency)
{
    targetFrequency = frequency;
}

void SineWaveGenerator::setVolume (float gainInDecibels)
{
    targetAmplitude = gainInDecibels;
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
    
    float freq = note->frequency;
    float ampl = note->gain;
    
    phaseIncrement = 2.0 * juce::MathConstants<float>::pi * freq / sampleRate;
    float amplitudeCompensation = std::pow (TILT, std::log2(freq / REFERENCE_FREQ));
    if (! applyCompensation)
        amplitudeCompensation = 1.0f;
    
    float noteGain = juce::Decibels::decibelsToGain (ampl + 8.0f);
    amplitudeCompensation *= noteGain;
    
    leftAmplitudeCompensation = amplitudeCompensation;
    rightAmplitudeCompensation = amplitudeCompensation;
    
    // Apply panning
    leftAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / -2.0);
    rightAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / 2.0);
}
