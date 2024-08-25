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
    SequenceableNote (float frequency, float amplitude, float pan,
                      float duration, StereoGainEnvelope envelope = StereoGainEnvelope());
    SequenceableNote (Note note, float duration, StereoGainEnvelope envelope = StereoGainEnvelope());
    
    const float getFrequency() const;
    const float getAmplitude() const;
    const float getPan() const;
    const float getDuration() const;
    
    void setFrequency (float newFrequency);
    void setAmplitude (float newAmplitude);
    void setPan (float newPan);
    void setDuration (float newDuration);
    
    const std::pair<float, float> getGainAtSample (int sample) const;
    
    const Note getNote() const;
    
    const SequenceableNote withAmplitudeChange (float amplChange) const; // returns a copy of this with varying ampl
    const SequenceableNote withPan (float pan) const; // returns a copy of this, with panning added to the envelope
    
    
private:
    Note note;
    float duration;
    
    StereoGainEnvelope envelope;
};
