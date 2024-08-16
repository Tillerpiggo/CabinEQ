/*
  ==============================================================================

    CabinEQValueTreeManager.h
    Created: 6 Aug 2024 5:56:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEQValueTree.h"

// Manages multiple profiles of value trees.
class CabinEQValueTreeManager
{
public:
    CabinEQValueTreeManager (juce::AudioProcessorValueTreeState& apvts);
    
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void initProfiles(); // Initializes the profiles using the apvts
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEQValueTree>> getProfileNamed (juce::String profileName) const;
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<CabinEQValueTree>> profiles;
};
