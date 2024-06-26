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

float SineWaveGenerator::getNextSample()
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
    
    float sample = std::sin(phase) * amplitudeCompensation * gainRampCompensation;
    
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<float>::pi)
        phase -= 2.0 * juce::MathConstants<float>::pi;
    
    return sample;
}

// Sets the note to the newNote, and also "plays" it by ending the last note
// and starting the new note with a gain ramp
void SineWaveGenerator::setNote (Note newNote)
{
    if (! note)
    {
        std::cout << "new note: " << newNote.frequency << std::endl;
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

// ============================================
void SineWaveGenerator::updatePhaseIncrementAndAmplitudeCompensation()
{
    if (! note)
    {
        throw std::runtime_error("updatePhaseIncrementAndAmplitudeCompensation() called in SineWaveGenerator before setting the note to be played");
    }
    
    phaseIncrement = 2.0 * juce::MathConstants<float>::pi * note->frequency / sampleRate;
    amplitudeCompensation = std::pow(TILT, std::log2(note->frequency / REFERENCE_FREQ));
    amplitudeCompensation *= juce::Decibels::decibelsToGain (note->gain);
}
