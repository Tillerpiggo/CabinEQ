/*
  ==============================================================================

    CabinEqProfileManager.h
    Created: 10 Oct 2024 7:51:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqProfile.h"

// Managed multiple profiles of band EQ settings.
class CabinEqProfileManager
{
public:
    CabinEqProfileManager (juce::AudioProcessorValueTreeState& apvts);
    
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void renameProfile (juce::String profileName, juce::String newProfileName);
    void setProfileVolume (juce::String profileName, float profileVolume);
    void setProfileMelodyVolume (juce::String profileName, float melodyVolume);
    void setProfileNoiseVolume (juce::String profileName, float noiseVolume);
    void initProfiles();
    void lockAllProfiles();
    
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEqProfile>> getProfileNamed (juce::String profileName) const;
    std::optional<juce::String> getLastSelectedProfileName() const;
    float getMasterVolume() const;
    bool getHasLicense() const;
    
    void setLastSelectedProfileName (juce::String lastSelectedProfileName);
    void setMasterVolume (float masterVolume);
    void setHasLicense (bool hasLicense);
    
private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<CabinEqProfile>> profiles; // must use unique ptr because the copy operator is implicitly deleted
    
    juce::Identifier lastSelectedProfileId { "lastSelectedProfileId" };
    juce::Identifier masterVolumeId { "masterVolumeId" };
    juce::Identifier hasLicenseId { "hasLicenseId" };
};
