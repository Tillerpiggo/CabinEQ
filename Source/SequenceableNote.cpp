/*
  ==============================================================================

    SequenceableNote.cpp
    Created: 5 Jul 2024 4:03:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SequenceableNote.h"

SequenceableNote::SequenceableNote (float frequency, float amplitude,
                  float duration, StereoGainEnvelope envelope)
    : frequency (frequency), amplitude (amplitude),
      duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (Note note, float duration, StereoGainEnvelope envelope)
    : frequency (note.frequency), amplitude (note.gain),
      duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (EQNode node, float duration, StereoGainEnvelope envelope)
    : frequency (node.frequency), amplitude (node.amplitude),
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

const std::pair<float, float> SequenceableNote::getGainAtSample (int sample) const
{
    return envelope.getGainAtSample (sample, duration);
}

const Note SequenceableNote::note() const
{
    return Note (frequency, amplitude);
}
