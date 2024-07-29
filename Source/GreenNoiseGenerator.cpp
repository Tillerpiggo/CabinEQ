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
    float sample = arbitrarySequencer.getNextSample().first * 0.15f + random.nextFloat() * 0.25f;
   // std::cout << "sample: " << sample << std::endl;
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

float GreenNoiseGenerator::getCurrFreq() const
{
    float freq = arbitrarySequencer.currentlyPlayingFrequency();
    if (freq < 0)
        return 1000.0f;
    return freq;
}

void GreenNoiseGenerator::updateArbitrarySequencerNotes()
{
    std::vector<SequenceableNote> notes;
    float noteFactor = 0.96;
    float numNotes = 5;
    
    for (int i = 0; i < numNotes; ++i)
    {
        float freq = centerFreq * std::pow (noteFactor, i - numNotes / 2);
        SequenceableNote note (freq, 0.0f, 0.0f, 0.0f, 5000, StereoGainEnvelope (500, 0));
        notes.push_back (note);
    }
    
    for (int i = 1; i < numNotes - 1; ++i)
    {
        float freq = centerFreq * std::pow (noteFactor, numNotes / 2 - i);
        SequenceableNote note (freq, 0.0f, 0.0f, 0.0f, 5000, StereoGainEnvelope (500, 0));
        notes.push_back (note);
    }
    
    arbitrarySequencer.setNotes (notes);
}
