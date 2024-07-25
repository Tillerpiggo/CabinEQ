/*
  ==============================================================================

    CurveValueTree.h
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SetPoint.h"

/// This class wraps and provides helper methods on a ValueTree to persist the set points of a single profile. It should be used to initially load and save
/// this data, rather than to actively manage it.
class ClearEQValueTree
{
public:
    ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier);
    
    const std::vector<SetPoint> getSetPoints() const; // constructs set points matching the set points we have in memory
    
    void addNode (const float frequency, const float amplitude, const float pan);
    void removeNode (const float frequency);
    void resetNodes (const std::vector<SetPoint>& setPoints); // makes this value tree store the given set points
    
private:
    void resetAPVTS (juce::AudioProcessorValueTreeState& apvts);
    
    juce::Identifier idProfile, idSetPoint, idId, idFrequency, idAmplitude, idPan;
    juce::ValueTree valueTree;
};
