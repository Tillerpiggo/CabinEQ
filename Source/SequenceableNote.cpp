/*
  ==============================================================================

    SequenceableNote.cpp
    Created: 5 Jul 2024 4:03:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SequenceableNote.h"

SequenceableNote::SequenceableNote (float frequency, float amplitude, float pan, float phase,
                  float duration, StereoGainEnvelope envelope)
    : frequency (frequency), amplitude (amplitude), pan (pan), phase (phase),
      duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (Note note, float duration, StereoGainEnvelope envelope)
    : frequency (note.frequency), amplitude (note.gain), pan (note.pan), phase (note.phase),
      duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (EQNode node, float duration, StereoGainEnvelope envelope)
    : frequency (node.frequency), amplitude (node.amplitude), pan (node.pan), phase (0.0f),
      duration (duration), envelope (envelope)
{}

const float SequenceableNote::getFrequency() const
{
    return frequency;
}

const float SequenceableNote::getAmplitude() const
{
    return amplitude;
}

const float SequenceableNote::getPan() const
{
    return pan;
}

const float SequenceableNote::getPhase() const
{
    return phase;
}

const float SequenceableNote::getDuration() const
{
    return duration;
}

void SequenceableNote::setFrequency (float newFrequency)
{
    frequency = newFrequency;
}

void SequenceableNote::setAmplitude (float newAmplitude)
{
    amplitude = newAmplitude;
}

void SequenceableNote::setPan (float newPan)
{
    pan = newPan;
}

const std::pair<float, float> SequenceableNote::getGainAtSample (int sample) const
{
    return envelope.getGainAtSample (sample, duration);
}

const Note SequenceableNote::note() const
{
    return Note (frequency, amplitude, pan, phase);
}
