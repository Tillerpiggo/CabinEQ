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
  duration (duration), envelope (envelope) {}

SequenceableNote::SequenceableNote (Note note, float duration, StereoGainEnvelope envelope)
: frequency (note.frequency), amplitude (note.gain), pan (note.pan), phase (note.phase),
  duration (duration), envelope (envelope) {}

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

const std::pair<float, float> SequenceableNote::getGainAtSample (int sample) const
{
    return envelope.getGainAtSample (sample, duration);
}
