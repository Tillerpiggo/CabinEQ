/*
  ==============================================================================

    CurveValueTree.h
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EQNode.h"

/// This class wraps and provides helper methods on a ValueTree to persist the set points of a single profile. It should be used to initially load and save
/// this data, rather than to actively manage it.
class ClearEQValueTree
{
public:
    ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier);
    
    const std::vector<EQNode> getEQNodes() const; // constructs set points matching the set points we have in memory
    
    void addNode (const int id, const float frequency, const float amplitude, const float pan);
    int addNode (const float frequency, const float amplitude, const float pan); // returns id of new node
    void removeNode (const int id);
    void updateNode (const int id, const float frequency, const float amplitude, const float pan);
    void resetNodes (const std::vector<EQNode>& eqNodes); // makes this value tree store the given set points
    
    void initValueTreeFromAPVTS(); // sets value tree to match the one in apvts
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    
    void resetAPVTS (juce::AudioProcessorValueTreeState& apvts);
    void printValueTree (juce::ValueTree& valueTree) const;
    
    juce::Identifier idProfile, idEQNode, idId, idFrequency, idAmplitude, idPan;
    juce::ValueTree valueTree;
};
