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
    
    int addMultiBandStep (const bool isEnabled = true);
    void removeMultiBandStep (const int stepId);
    void setStepEnabled (const int stepId, const bool isEnabled);
    int addBand (const float freq, const float ampl, const float bandwidth, const Band::Type type, const int stepId); // returns the id of the added band
    void removeBand (const int bandId, const int stepId);
    void updateBand (const int bandId, const float freq, const float ampl, const float bandwidth, const Band::Type type, const int stepId);
    
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
    void addBandToMultiBandStep (int id, float freq, float ampl, float bandwidth, Band::Type type, juce::ValueTree multiBandStep);
    void updateBandInMultiBandStep (int id, float freq, float ampl, float bandwidth, Band::Type type, juce::ValueTree multiBandStep);
    int getNextIdInValueTree (juce::ValueTree valueTree); // returns the next id, i.e. the id the next added band would have, assuming the children of this node have sequential ids (deletions may cause id gaps, but this is fine)
    void printMultiBandStep (juce::ValueTree multiBandStep) const; // prints a ValueTree, assuming the ValueTree's children are Bands
    std::vector<Band> getBandsForMultiBandStep (juce::ValueTree multiBandStep) const;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::Identifier idProfile { "Profile" }; // the id/type name of the entire CabinEqProfile value tree
    juce::Identifier idProfileName { "ProfileName" }; // a property on value tree that stores the string name the user gave it
    juce::Identifier idProfileVolume { "ProfileVolume" };
    juce::Identifier idMelodyVolume { "MelodyVolume" };
    juce::Identifier idNoiseVolume { "NoiseVolume" };
    juce::Identifier idMultiBandStep { "MultiBandStep" }; // a step consisting of a list of bands and some other properties (enabled, etc.)
    juce::Identifier idEnabled { "Enabled" };
    juce::Identifier idBand { "Band" };
    juce::Identifier idId { "id" };
    juce::Identifier idFreq { "freq" };
    juce::Identifier idAmpl { "ampl" };
    juce::Identifier idBandwidth { "bandwidth" };
    juce::Identifier idBandType { "bandtype" };
    juce::Identifier idAmplTree { "AmplTree" };
    juce::ValueTree valueTree;
    juce::String profileName;
    float profileVolume = 0;
    float melodyVolume = 0;
    float noiseVolume = 0;
    
    bool hasBeenInitialized = false;
};
