/*
  ==============================================================================

    SequenceableNote.cpp
    Created: 5 Jul 2024 4:03:39pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "SequenceableNote.h"

SequenceableNote::SequenceableNote (float frequency, float amplitude, float pan,
                  float duration, StereoGainEnvelope envelope)
    : note (frequency, amplitude, pan), duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (Note note, float duration, StereoGainEnvelope envelope)
    : note (note), duration (duration), envelope (envelope)
{}

const float SequenceableNote::getFrequency() const
{
    return note.frequency;
}

const float SequenceableNote::getAmplitude() const
{
    return note.amplitude;
}

const float SequenceableNote::getPan() const
{
    return note.pan;
}

const float SequenceableNote::getDuration() const
{
    return duration;
}

void SequenceableNote::setFrequency (float newFrequency)
{
    this->note.frequency = newFrequency;
}

void SequenceableNote::setAmplitude (float newAmplitude)
{
    this->note.amplitude = newAmplitude;
}

void SequenceableNote::setPan (float newPan)
{
    this->note.pan = newPan;
}

void SequenceableNote::setDuration (float newDuration)
{
    this->duration = newDuration;
}

const std::pair<float, float> SequenceableNote::getGainAtSample (int sample) const
{
    return envelope.getGainAtSample (sample, duration);
}

const Note SequenceableNote::getNote() const
{
    return note;
}

const SequenceableNote SequenceableNote::withAmplitudeChange (float amplChange) const
{
    return SequenceableNote (getNote().frequency, getNote().amplitude + amplChange, getNote().pan, getDuration(), envelope);
}

const SequenceableNote SequenceableNote::withPan (float pan) const
{
    return SequenceableNote (getNote(), getDuration(), envelope.withPan (pan));
}
