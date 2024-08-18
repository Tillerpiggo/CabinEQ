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
    
    void applyLeftCrossfeed (float crossfeedGain = 0.3f, float delayInMs = 6.0f);
    void applyRightCrossfeed (float crossfeedGain = 0.3f, float delayInMs = 6.0f);
    
    const float getFrequency() const;
    const float getAmplitude() const;
    const float getDuration() const;
    const float getCrossfeedGain() const;
    const float getCrossfeedDelayInMs() const;
    const Channel getCrossfeedChannel() const;
    
    void setFrequency (float newFrequency);
    void setAmplitude (float newAmplitude);
    void setDuration (float newDuration);
    void setCrossfeedGain (float crossfeedGain);
    void setCrossfeedDelayInMs (float delayInMs);
    void setCrossfeedChannel (Channel channel);
    
    const std::pair<float, float> getGainAtSample (int sample) const;
    
    const Note getNote() const;
    
private:
    Note note;
    float duration;
    
    StereoGainEnvelope envelope;
};
