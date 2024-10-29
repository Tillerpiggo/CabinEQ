/*
  ==============================================================================

    MelodicNoiseSequencer.cpp
    Created: 27 Oct 2024 1:05:16pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "MelodicNoiseSequencer.h"
#include <random>

MelodicNoiseSequencer::MelodicNoiseSequencer()
{
    
}

void MelodicNoiseSequencer::prepare (const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = spec.sampleRate;
    lowPassFilter.prepare (spec);
    highPassFilter.prepare (spec);
    bandpassFilter.prepare (spec);
    sineWaveGenerator.setSampleRate (spec.sampleRate);
    spatialPinkNoiseGenerator.setSampleRate (spec.sampleRate);
}

void MelodicNoiseSequencer::setPattern (std::vector<NoiseNote> notes)
{
    this->notes = notes;
    currNoteIdx = 0;
    numSamplesNoteHasBeenPlaying = 0;
    updateFilters();
}

void MelodicNoiseSequencer::setSineVolume (float sineVolume)
{
    this->sineVolume = sineVolume;
}

void MelodicNoiseSequencer::setSpeedFactor (float speedFactor)
{
    this->speedFactor = speedFactor;
}

void MelodicNoiseSequencer::setOctaveRange (float octaveRange)
{
    this->octaveRange = octaveRange;
}

std::pair<float, float> MelodicNoiseSequencer::getNextSample()
{
    // If we don't have notes, return nothing
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return { 0.0f, 0.0f };
    
    float lowerNoiseSample = lowerNoise.generate();
    float upperNoiseSample = upperNoise.generate();
    float bandpassSample = noteNoise.generate();
    
    if (getCurrNote().bandwidth == 0)
    {
        bandpassSample = 0;
    }
    
    lowerNoiseSample = lowPassFilter.processSample (lowerNoiseSample);
    upperNoiseSample = highPassFilter.processSample (upperNoiseSample);
    bandpassSample = bandpassFilter.processSample (bandpassSample);
    
    numSamplesNoteHasBeenPlaying++;
    if (numSamplesNoteHasBeenPlaying >= getCurrNote().durationInSamples * speedFactor)
        goToNextNote();
    
    float envelopeGain = getCurrNote().getGainAtSample (numSamplesNoteHasBeenPlaying).first;
    
    if (snapToZeroCounter >= 1000)
    {
        lowPassFilter.snapToZero();
        highPassFilter.snapToZero();
        bandpassFilter.snapToZero();
        snapToZeroCounter = 0;
    }
    snapToZeroCounter++;
    
//    float sample = noiseSample * 10.0f + sineSample * 0.15f * juce::Decibels::decibelsToGain (sineVolume) * envelopeGain;
    float sample = lowerNoiseSample * 10.0f + upperNoiseSample * 10.0f + bandpassSample * 10.0f * juce::Decibels::decibelsToGain (sineVolume) * envelopeGain;
//    float sample = bandpassSample * 10.0f * juce::Decibels::decibelsToGain (sineVolume) * envelopeGain;
    return { sample, sample };
    
}

NoiseNote MelodicNoiseSequencer::getCurrNote()
{
    if (currNoteIdx < 0 || currNoteIdx >= notes.size())
        return notes[0];
    return notes[currNoteIdx];
}

void MelodicNoiseSequencer::updateFilters()
{
    
    // Update the high and low pass filters
    float freqFactor = 2.5f; // factor above and below center that we place the high/low pass filters
    float q = 3.0f;
    float freq = getCurrNote().freqFactor * freqOffsetFactor; // assume this is absolute, not relative
    if (freq * freqFactor >= sampleRate * 0.49 || freq / freqFactor <= 10)
        return;
    
    if (getCurrNote().bandwidth == 0)
    {
        return;
    }
    else
    {
        *lowPassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, freq / freqFactor, q);
        *highPassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq * freqFactor, q);
        *bandpassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, freq, 1.0f);
    }
}

void MelodicNoiseSequencer::goToNextNote()
{
    numSamplesNoteHasBeenPlaying = 0;
    currNoteIdx++;
    
    if (currNoteIdx >= notes.size())
        currNoteIdx = 0;
    
    // Generate random octave offset from -5 and 5
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(-std::round (octaveRange), std::round (octaveRange));
    int octaveOffset = distr(gen);
    
    freqOffsetFactor = std::pow (2.0f, static_cast<float> (octaveOffset));
    
    float noteFreq = getCurrNote().freqFactor * freqOffsetFactor;
    while (noteFreq < 20.0f)
    {
        freqOffsetFactor *= 2.0f;
        noteFreq = getCurrNote().freqFactor * freqOffsetFactor;
    }
    while (noteFreq > 17000.0f)
    {
        freqOffsetFactor /= 2.0f;
        noteFreq = getCurrNote().freqFactor * freqOffsetFactor;
    }
    
    sineWaveGenerator.setNote (Note (getCurrNote().freqFactor * freqOffsetFactor, 0.0f, 0.0f, 0.0f));
    
    updateFilters();
}
