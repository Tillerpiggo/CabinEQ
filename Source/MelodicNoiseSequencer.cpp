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
    std::cout << "prepared notch filter" << std::endl;
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
    
    noiseSample = notchFilter.processSample (noiseSample);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples)
        goToNextNote();
    
    float envelopeGain = getCurrNote().getGainAtSample (numSamplesNoteHasBeenPlaying).first;
    
    float sample = noiseSample + sineSample * envelopeGain;
    return { sample, sample };
    
}

NoiseNote MelodicNoiseSequencer::getCurrNote()
{
    std::cout << "get curr note" << std::endl;
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return notes[0];
    return notes[currNoteIdx];
}

void MelodicNoiseSequencer::updateNotchFilter()
{
    float freq = getCurrNote().freqFactor; // assume this is absolute, not relative
    std::cout << "attempting to make notch filter with sampleRate " << sampleRate << ", freq " << freq << ", Q factor 0.5f" << std::endl;
    *notchFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, freq, 0.5f);
    std::cout << "made notch filter" << std::endl;
}

void MelodicNoiseSequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
        currNoteIdx = 0;
    
    updateNotchFilter();
}
