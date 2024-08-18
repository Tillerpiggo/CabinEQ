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
#include "EQNode.h"

class SequenceableNote
{
public:
    SequenceableNote (float frequency, float amplitude,
                      float duration, StereoGainEnvelope envelope = StereoGainEnvelope());
    SequenceableNote (Note note, float duration, StereoGainEnvelope envelope = StereoGainEnvelope());
    SequenceableNote (EQNode node, float duration, StereoGainEnvelope envelope = StereoGainEnvelope());
    
    const float getFrequency() const;
    const float getAmplitude() const;
    const float getDuration() const;
    
    void setFrequency (float newFrequency);
    void setAmplitude (float newAmplitude);
    
    const std::pair<float, float> getGainAtSample (int sample) const;
    
    const Note note() const;
    
private:
    float frequency;
    float amplitude;
    float duration;
    
    StereoGainEnvelope envelope;
};
