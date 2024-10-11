/*
  ==============================================================================

    CabinEqProfileManager.cpp
    Created: 10 Oct 2024 7:51:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqProfileManager.h"

CabinEqProfileManager::CabinEqProfileManager (juce::AudioProcessorValueTreeState& apvts)
    : apvts (apvts)
{}

void CabinEqProfileManager::addProfile (juce::String profileName)
{
    // Create the profile
    if (getProfileNamed (profileName) == std::nullopt)
    {
        auto newProfile = std::make_unique<CabinEqProfile> (apvts, profileName);
        profiles.push_back (std::move (newProfile));
    }
}

void CabinEqProfileManager::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    addProfile (profileName);
    
    // Copy over old profile to new profile
    getProfileNamed (profileName)->get().copyFrom (getProfileNamed (oldProfileName)->get());
}

void CabinEqProfileManager::removeProfile (juce::String profileName)
{
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            profiles.erase (profiles.begin() + i);
}

void CabinEqProfileManager::renameProfile (juce::String profileName, juce::String newProfileName)
{
    getProfileNamed (profileName)->get().renameTo (newProfileName);
}

void CabinEqProfileManager::initProfiles()
{
    for (const auto& node: apvts.state)
    {
        if (node.getType().toString() == "Profile")
        {
            profiles.push_back (std::make_unique<CabinEqProfile> (apvts, node.getProperty ("ProfileName")));
        }
    }
    
    for (auto& profile : profiles)
        profile->initValueTreeFromAPVTS();
}

const std::vector<juce::String> CabinEqProfileManager::getProfileNames() const
{
    std::vector<juce::String> profileNames;
    for (const auto& profile : profiles)
        profileNames.push_back (profile->getName());
    return profileNames;
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqProfileManager::getProfileNamed (juce::String profileName) const
{
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            return std::ref (*profiles[i]);
    return std::nullopt;
}

std::optional<juce::String> CabinEqProfileManager::getLastSelectedProfileName() const
{
    if (apvts.state.hasProperty (lastSelectedProfileId))
    {
        return apvts.state.getProperty (lastSelectedProfileId);
    }
    return std::nullopt;
}

void CabinEqProfileManager::setLastSelectedProfileName (juce::String lastSelectedProfileName)
{
    apvts.state.setProperty (lastSelectedProfileId, lastSelectedProfileName, nullptr);
}
