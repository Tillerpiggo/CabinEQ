/*
  ==============================================================================

    MelodicNoiseSequencer.cpp
    Created: 27 Oct 2024 1:05:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MelodicNoiseSequencer.h"


MelodicNoiseSequencer::MelodicNoiseSequencer()
{
    
}

void MelodicNoiseSequencer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    notchFilter.prepare (spec);
    sineWaveGenerator.setSampleRate (spec.sampleRate);
    spatialPinkNoiseGenerator.setSampleRate (spec.sampleRate);
    spatialPinkNoiseGenerator.setBandpass (1000.0f, 0.5f);
}

void MelodicNoiseSequencer::setPattern (std::vector<NoiseNote> notes)
{
    this->notes = notes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    updateNotchFilter();
}

std::pair<float, float> MelodicNoiseSequencer::getNextSample()
{
    // If we don't have notes, return nothing
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return { 0.0f, 0.0f };
    
    float noiseSample = pinkNoise.generate();
    float sineSample = sineWaveGenerator.getNextSample().first;
    sineSample = spatialPinkNoiseGenerator.getNextSample().first;
    
    noiseSample = notchFilter.processSample (noiseSample);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples)
        goToNextNote();
    
    float envelopeGain = getCurrNote().getGainAtSample (numSamplesNoteHasBeenPlaying).first;
    
    if (snapToZeroCounter >= 1000)
    {
        notchFilter.snapToZero();
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
//    float sample = noiseSample * 15.0f + sineSample * 0.5f * envelopeGain;f
    float sample = noiseSample * 15.0f + sineSample * 45.0f * envelopeGain;
    return { sample, sample };
    
}

NoiseNote MelodicNoiseSequencer::getCurrNote()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return notes[0];
    return notes[currNoteIdx];
}

void MelodicNoiseSequencer::updateNotchFilter()
{
    float freq = getCurrNote().freqFactor; // assume this is absolute, not relative
    if (freq >= sampleRate * 0.49)
        return;
    *notchFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, freq, 0.5f);
    spatialPinkNoiseGenerator.setBandpass (freq, 0.5f);
}

void MelodicNoiseSequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
        currNoteIdx = 0;
    
    sineWaveGenerator.setNote (Note (getCurrNote().freqFactor, 0.0f, 0.0f, 0.0f));
    
    updateNotchFilter();
}
