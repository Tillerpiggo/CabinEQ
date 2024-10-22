/*
  ==============================================================================

    CabinEqProfile.h
    Created: 10 Oct 2024 7:51:38pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BandProfile.h"

/// This class wraps and provides helper methods on a ValueTree to persist the parametric EQ bands of a single profile. It can be used to interface with the persistent layer to read and write this data.
class CabinEqProfile
{
public:
    CabinEqProfile (juce::AudioProcessorValueTreeState& apvts, const juce::String identifier);
    
    const BandProfile getBandProfile() const; // constructs a list of bands matching the ones in memory in the valueTree. For now, we're only doing amplitude bands, so it implicitly uses the amplTree. Uses whatever the current profileVolume is (assumed to be up to date)
    const std::optional<Band> getBandWithId (const int id) const;
    
    int addBand (const float freq, const float ampl, const float bandwidth);
    void removeBand (const int id);
    void updateBand (const int id, const float freq, const float ampl, const float bandwidth);
    
    void initValueTreeFromAPVTS(); // sets this value tree to match the one in the main apvts
    const juce::String getName() const;
    const float getVolume() const;
    const float getMelodyVolume() const;
    const float getNoiseVolume() const;
    
    void copyFrom (CabinEqProfile other);
    void renameTo (juce::String newName);
    void setVolume (float profileVolume);
    void setMelodyVolume (float melodyVolume);
    void setNoiseVolume (float noiseVolume);
    
private:
    void addBandToTree (int id, float freq, float ampl, float bandwidth, juce::ValueTree bandTree);
    void updateBandInTree (int id, float freq, float ampl, float bandwidth, juce::ValueTree bandTree);
    int getNextIdForBandInTree (juce::ValueTree bandTree); // returns the next id, i.e. the id the next added band would have, for this band tree
    void printBandTree (juce::ValueTree bandTree) const; // prints a ValueTree, assuming the ValueTree's children are Bands
    std::vector<Band> getBandsForValueTree (juce::ValueTree valueTree) const;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::Identifier idProfile { "Profile" }; // the id/type name of the entire CabinEqProfile value tree
    juce::Identifier idProfileName { "ProfileName" }; // a property on value tree that stores the string name the user gave it
    juce::Identifier idProfileVolume { "ProfileVolume" };
    juce::Identifier idMelodyVolume { "MelodyVolume" };
    juce::Identifier idNoiseVolume { "NoiseVolume" };
    juce::Identifier idBand { "Band" };
    juce::Identifier idId { "id" };
    juce::Identifier idFreq { "freq" };
    juce::Identifier idAmpl { "ampl" };
    juce::Identifier idBandwidth { "bandwidth" };
    juce::Identifier idAmplTree { "AmplTree" };
    juce::ValueTree valueTree;
    juce::String profileName;
    float profileVolume = 0;
    float melodyVolume = 0;
    float noiseVolume = 0;
    
    bool hasBeenInitialized = false;
};
