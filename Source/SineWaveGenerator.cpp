/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

SineWaveGenerator::SineWaveGenerator()
{
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
    
    // Vibrato
    vibratoPhase += vibratoStep;
    if (vibratoPhase < 0 || vibratoPhase > 2 * M_PI)
        vibratoPhase = 0;
    float vibratoChange = (vibratoMaxDB - vibratoMinDB) * (std::sin (vibratoPhase) / 2.0 + 0.5) + vibratoMinDB;
    vibratoChange = juce::Decibels::decibelsToGain (vibratoChange);
    
    float leftSample = std::sin (phase) * leftAmplitudeCompensation * vibratoChange;
    float rightSample = std::sin (phase + note->phase) * rightAmplitudeCompensation * vibratoChange;
    
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

void SineWaveGenerator::setFrequency (float frequencyInHz)
{
    targetFrequency = frequencyInHz;
}

void SineWaveGenerator::setVolume (float volumeInDecibels)
{
    targetAmplitude = volumeInDecibels;
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
    
    float noteGain = juce::Decibels::decibelsToGain (ampl + 6.0f);
    leftAmplitudeCompensation = noteGain;
    rightAmplitudeCompensation = noteGain;
    
    // Apply panning
    leftAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / -2.0);
    rightAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / 2.0);
}
