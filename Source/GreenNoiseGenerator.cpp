/*
  ==============================================================================

    GreenNoiseGenerator.cpp
    Created: 29 Jul 2024 2:43:35pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "GreenNoiseGenerator.h"

GreenNoiseGenerator::GreenNoiseGenerator()
    : arbitrarySequencer (false)
{}

std::pair<float, float> GreenNoiseGenerator::getNextSample()
{
    float sample = arbitrarySequencer.getNextSample().first + random.nextFloat() * 0.25f;
    return { sample, sample };
}

void GreenNoiseGenerator::setSampleRate (float newSampleRate)
{
    arbitrarySequencer.setSampleRate (newSampleRate);
}

void GreenNoiseGenerator::setCenterFrequency (float centerFreq)
{
    this->centerFreq = centerFreq;
    updateArbitrarySequencerNotes();
}

float GreenNoiseGenerator::getCenterFrequency() const
{
    return centerFreq;
}

void GreenNoiseGenerator::updateArbitrarySequencerNotes()
{
    std::vector<SequenceableNote> notes;
    float noteFactor = 0.99;
    float numNotes = 5;
    
    for (int i = 0; i < numNotes; ++i)
    {
        float freq = std::pow (noteFactor, i - numNotes / 2);
        SequenceableNote note (freq, 0.0f, 0.0f, 0.0f, 20000, StereoGainEnvelope());
        notes.push_back (note);
    }
    
    arbitrarySequencer.setNotes (notes);
}
