/*
  ==============================================================================

    SequenceableNote.h
    Created: 5 Jul 2024 3:59:49pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Note.h"
#include "StereoGainEnvelope.h"

class SequenceableNote
{
public:
    SequenceableNote (float frequency, float amplitude, float pan, float phase,
                      float duration, StereoGainEnvelope envelope);
    SequenceableNote (Note note, float duration, StereoGainEnvelope envelope);
    
    const float getFrequency() const;
    const float getAmplitude() const;
    const float getPan() const;
    const float getPhase() const;
    const float getDuration() const;
    const std::pair<float, float> getGainAtSample (int sample) const;
    
private:
    float frequency;
    float amplitude;
    float pan;
    float phase;
    float duration;
    
    StereoGainEnvelope envelope;
};
