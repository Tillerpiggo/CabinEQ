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
#include "Curve.h"

/// This class wraps and provides helper methods on a ValueTree to persist the set points of a single profile. It should be used to initially load and save
/// this data, rather than to actively manage it.
class CabinEQValueTree
{
public:
    CabinEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier);
    
    const std::vector<EQNode> getEQNodes() const; // constructs set points matching the set points we have in memory
    const std::optional<EQNode> getEQNodeWithId (const int id) const;
    Curve& getCurve();
    
    void addEQNode (const int id, const float frequency, const float amplitude, const float pan);
    int addEQNode (const float frequency, const float amplitude, const float pan); // returns id of new node
    void removeEQNode (const int id);
    void updateEQNode (const int id, const float frequency, const float amplitude, const float pan);
    void resetNodes (const std::vector<EQNode>& eqNodes); // makes this value tree store the given set points
    
    void initValueTreeFromAPVTS(); // sets value tree to match the one in apvts
    const juce::String getName() const;
    
    void copyFrom (CabinEQValueTree& other);
    
private:
    void resetAPVTS (juce::AudioProcessorValueTreeState& apvts);
    void printValueTree (juce::ValueTree valueTree) const;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::Identifier idProfile, idEQNode, idId, idFrequency, idAmplitude, idPan;
    juce::ValueTree valueTree;
    
    Curve curve;
    bool hasBeenInitialized = false;
};
