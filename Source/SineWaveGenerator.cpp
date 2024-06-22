/*
  ==============================================================================

    SineWaveGenerator.cpp
    Created: 14 Jun 2024 3:52:54pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SineWaveGenerator.h"

void SineWaveGenerator::setSampleRate (double newSampleRate)
{
    sampleRate = newSampleRate;
}

double SineWaveGenerator::getNextSample()
{
    double sample = std::sin(phase) * amplitudeCompensation;
    
    phase += phaseIncrement;
    if (phase > 2.0 * juce::MathConstants<double>::pi)
        phase -= 2.0 * juce::MathConstants<double>::pi;
    
    return sample;
}

// Sets the note to the newNote, and also "plays" it by ending the last note
// and starting the new note with a gain ramp
void SineWaveGenerator::setNote (Note newNote)
{
    // TODO: Fancy gain ramp stuff. RN this is just a hard switch
    note = newNote;
    updatePhaseIncrementAndAmplitudeCompensation();
}

void SineWaveGenerator::updatePhaseIncrementAndAmplitudeCompensation()
{
    if (!note)
    {
        throw std::runtime_error("updatePhaseIncrementAndAmplitudeCompensation() called in SineWaveGenerator before setting the note to be played");
    }
    
    std::cout << "Note frequency: " << note->frequency << std::endl;
    
    phaseIncrement = 2.0 * juce::MathConstants<double>::pi * note->frequency / sampleRate;
    amplitudeCompensation = std::pow(TILT, std::log2(note->frequency / REFERENCE_FREQ));
}
