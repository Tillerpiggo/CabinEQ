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
    float gainRampCompensation = 1.0f;
    if (endNoteGainRamp > 0)
    {
        gainRampCompensation = static_cast<float> (endNoteGainRamp) / static_cast<float> (GAIN_RAMP_LEN_IN_SAMPLES);
        endNoteGainRamp--;
    }
    else if (startNoteGainRamp > 0)
    {
        gainRampCompensation = 1.0f - static_cast<float> (startNoteGainRamp) / static_cast<float> (GAIN_RAMP_LEN_IN_SAMPLES);
        startNoteGainRamp--;
    }
    
    if (endNoteGainRamp == 0)
    {
        note = nextNote;
        nextNote.reset();
        updatePhaseIncrementAndAmplitudeCompensation();
        phase = 0;
        endNoteGainRamp--;
    }
    
    float leftSample = std::sin (phase) * leftAmplitudeCompensation * gainRampCompensation;
    float rightSample = std::sin (phase + note->phase) * rightAmplitudeCompensation * gainRampCompensation;
    
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<float>::pi)
        phase -= 2.0 * juce::MathConstants<float>::pi;
    
    return { leftSample, rightSample };
}

// Sets the note to the newNote, and also "plays" it by ending the last note
// and starting the new note with a gain ramp
void SineWaveGenerator::setNote (Note newNote)
{
    if (! note)
    {
        note = newNote;
        updatePhaseIncrementAndAmplitudeCompensation();
        return;
    }
    
    // TODO: Fancy gain ramp stuff. RN this is just a hard switch
    nextNote = newNote;
    endNoteGainRamp = GAIN_RAMP_LEN_IN_SAMPLES;
    startNoteGainRamp = GAIN_RAMP_LEN_IN_SAMPLES;
}

void SineWaveGenerator::setVolume (float gainInDecibels)
{
    // TODO: create gain ramp
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
    float noteGain = juce::Decibels::decibelsToGain (note->gain);
    amplitudeCompensation *= noteGain;
    
    leftAmplitudeCompensation = amplitudeCompensation;
    rightAmplitudeCompensation = amplitudeCompensation;
    
    // Apply panning
    if (note->pan < 20)
    {
        leftAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / -2.0);
        rightAmplitudeCompensation *= juce::Decibels::decibelsToGain (note->pan / 2.0);
    }
}
