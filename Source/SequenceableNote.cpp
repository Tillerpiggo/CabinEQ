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
    : note (frequency, amplitude), duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (Note note, float duration, StereoGainEnvelope envelope)
    : note (note), duration (duration), envelope (envelope)
{}

SequenceableNote::SequenceableNote (EQNode node, float duration, StereoGainEnvelope envelope)
    : note (node.frequency, node.amplitude), duration (duration), envelope (envelope)
{}

const float SequenceableNote::getFrequency() const
{
    return note.frequency;
}

const float SequenceableNote::getAmplitude() const
{
    return note.amplitude;
}

const float SequenceableNote::getDuration() const
{
    return duration;
}

const float SequenceableNote::getCrossfeedGain() const
{
    return note.crossfeedGain;
}

const float SequenceableNote::getCrossfeedDelayInMs() const
{
    return note.crossfeedDelayInMs;
}

const Channel SequenceableNote::getCrossfeedChannel() const
{
    return note.crossfeedChannel;
}

void SequenceableNote::setFrequency (float newFrequency)
{
    this->note.frequency = newFrequency;
}

void SequenceableNote::setAmplitude (float newAmplitude)
{
    this->note.amplitude = newAmplitude;
}

void SequenceableNote::setDuration (float newDuration)
{
    this->duration = newDuration;
}

void SequenceableNote::setCrossfeedGain (float crossfeedGain)
{
    this->note.crossfeedGain = crossfeedGain;
}

void SequenceableNote::setCrossfeedDelayInMs (float delayInMs)
{
    this->note.crossfeedDelayInMs = delayInMs;
}

void SequenceableNote::setCrossfeedChanenl (Channel channel)
{
    this->note.crossfeedChannel = channel;
}

const std::pair<float, float> SequenceableNote::getGainAtSample (int sample) const
{
    return envelope.getGainAtSample (sample, duration);
}

const Note SequenceableNote::getNote() const
{
    return note;
}
