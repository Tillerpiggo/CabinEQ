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
        if (targetAmplitude.value() > note->amplitude)
        {
            note->amplitude *= AMPL_STEP;
        }
        else
        {
            note->amplitude /= AMPL_STEP;
        }
        
        float ratio = note->amplitude / targetAmplitude.value();
        if (ratio < AMPL_STEP && ratio > 1.0f / AMPL_STEP)
        {
            targetAmplitude.reset();
        }
        updatePhaseIncrementAndAmplitudeCompensation();
    }
    
    if (targetPan.has_value())
    {
        if (targetPan.value() > note->pan)
        {
            note->pan *= AMPL_STEP;
        }
        else
        {
            note->pan /= AMPL_STEP;
        }
        
        float ratio = note->pan / targetPan.value();
        if (ratio < AMPL_STEP && ratio > 1.0f / AMPL_STEP)
        {
            targetPan.reset();
        }
        updatePhaseIncrementAndAmplitudeCompensation();
    }
    
    if (isTremoloing)
    {
        tremoloPan += tremoloStep;
//        std::cout << "tremoloStep: " << tremoloStep << std::endl;
//        std::cout << "tremoloPan: " << tremoloPan << std::endl;
        
        if (tremoloPan > 1)
        {
            tremoloStep = -0.0001;
        }
        if (tremoloPan < -1)
        {
            tremoloStep = 0.0001;
        }
        updatePhaseIncrementAndAmplitudeCompensation();
    }
    
    // Vibrato
//    vibratoPhase += vibratoStep;
//    if (vibratoPhase < 0 || vibratoPhase > 2 * M_PI)
//        vibratoPhase = 0;
//    float vibratoChange = (vibratoMaxDB - vibratoMinDB) * (std::sin (vibratoPhase) / 2.0 + 0.5) + vibratoMinDB;
//    vibratoChange = juce::Decibels::decibelsToGain (vibratoChange);
    
    float leftSample = std::sin (phase - 0.5 * note->phase) * leftAmplitudeCompensation;// * vibratoChange;
    float rightSample = std::sin (phase + 0.5 * note->phase) * rightAmplitudeCompensation;// * vibratoChange;
    
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
    targetPan.reset();
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
    targetPan = panInDecibels;
}

void SineWaveGenerator::setPhase (float phaseInRadians)
{
    targetPhase = phaseInRadians;
}

void SineWaveGenerator::startTremolo (float startPan)
{
    isTremoloing = true;
    tremoloPan = startPan;
}

// ============================================
void SineWaveGenerator::updatePhaseIncrementAndAmplitudeCompensation()
{
    if (! note)
    {
        throw std::runtime_error("updatePhaseIncrementAndAmplitudeCompensation() called in SineWaveGenerator before setting the note to be played");
    }
    
    float freq = note->frequency;
    float ampl = note->amplitude;
    float pan = note->pan;
    
    ampl += -3.0f * std::log2 (freq / 1000.0f); // cabin noise after calibration tilt
    
    phaseIncrement = 2.0 * juce::MathConstants<float>::pi * freq / sampleRate;
    
    float leftDB = ampl - (pan < 0 ? pan : 0);
    float rightDB = ampl + (pan > 0 ? pan : 0);
    
    // Tremolo
    float angle = tremoloPan * M_PI / 4.0f; // go from [-1, 1] to [-pi/4, pi/4]
    float leftGain = std::sqrt (2.0f) / 2.0f * (std::cos (angle) - std::sin(angle));
    float rightGain = std::sqrt (2.0f) / 2.0f * (std::cos(angle) + std::sin(angle));
    leftDB += juce::Decibels::gainToDecibels (leftGain);
    rightDB += juce::Decibels::gainToDecibels (rightGain);
    
    leftAmplitudeCompensation = juce::Decibels::decibelsToGain (leftDB);
    rightAmplitudeCompensation = juce::Decibels::decibelsToGain (rightDB);
}
