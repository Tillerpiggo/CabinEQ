/*
  ==============================================================================

    CabinEqValueTreeManager.cpp
    Created: 6 Aug 2024 5:56:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqValueTreeManager.h"

CabinEqValueTreeManager::CabinEqValueTreeManager (juce::AudioProcessorValueTreeState& apvts) 
    : apvts (apvts)
{}

void CabinEqValueTreeManager::addProfile (juce::String profileName)
{
    // Create the profile
    if (getProfileNamed (profileName) == std::nullopt)
    {
        auto newProfile = std::make_unique<CabinEqValueTree> (apvts, profileName);
        
        // Add starter points, logarithmically spaced
        int numPoints = 6;
        float paddingFactor = 1.04; // so the points aren't right up against the edge
        float minFreq = 20 * paddingFactor;
        float maxFreq = 20000 / paddingFactor;
        std::vector<float> freqs;
        
        for (int i = 0; i < numPoints; ++i)
        {
            float ratio = static_cast<float> (i) / (numPoints - 1);
            float logFreq = minFreq * std::pow (maxFreq / minFreq, ratio);
            freqs.push_back (logFreq);
        }
        
        for (const auto& freq : freqs)
        {
            newProfile->addAmplPt (freq, 0);
            newProfile->addPanPt (freq, 0);
            newProfile->addPhasePt (freq, 0);
        }
        
        profiles.push_back (std::move (newProfile));
    }
    
}

void CabinEqValueTreeManager::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    addProfile (profileName);
    
    // Copy over old profile to new profile
    getProfileNamed (profileName)->get().copyFrom (getProfileNamed (oldProfileName)->get());
}

void CabinEqValueTreeManager::removeProfile (juce::String profileName)
{
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            profiles.erase (profiles.begin() + i);
}

void CabinEqValueTreeManager::renameProfile (juce::String profileName, juce::String newProfileName)
{
    getProfileNamed (profileName)->get().renameTo (newProfileName);
}

void CabinEqValueTreeManager::initProfiles()
{
    for (const auto& node : apvts.state)
    {
        if (node.getType().toString() == "Profile")
        {
            profiles.push_back (std::make_unique<CabinEqValueTree> (apvts, node.getProperty ("ProfileName")));
        }
    }
        
    
    for (const auto& profile : profiles)
        profile->initValueTreeFromAPVTS();
}

const std::vector<juce::String> CabinEqValueTreeManager::getProfileNames() const
{
    std::vector<juce::String> profileNames;
    for (const auto& profile : profiles)
        profileNames.push_back (profile->getName());
    return profileNames;
}

std::optional<std::reference_wrapper<CabinEqValueTree>> CabinEqValueTreeManager::getProfileNamed (juce::String profileName) const
{
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            return std::ref (*profiles[i]);
    return std::nullopt;
}

std::optional<juce::String> CabinEqValueTreeManager::getLastSelectedProfileName() const
{
    if (apvts.state.hasProperty (lastSelectedProfileId))
    {
        return apvts.state.getProperty (lastSelectedProfileId);
    }
    return std::nullopt;
}

void CabinEqValueTreeManager::setLastSelectedProfileName (juce::String lastSelectedProfileName)
{
    apvts.state.setProperty (lastSelectedProfileId, lastSelectedProfileName, nullptr);
}
