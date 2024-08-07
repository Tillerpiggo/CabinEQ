/*
  ==============================================================================

    CabinEQValueTreeManager.cpp
    Created: 6 Aug 2024 5:56:08pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQValueTreeManager.h"

CabinEQValueTreeManager::CabinEQValueTreeManager (juce::AudioProcessorValueTreeState& apvts) 
    : apvts (apvts)
{}

void CabinEQValueTreeManager::addProfile (juce::String profileName)
{
    if (getProfileNamed (profileName) == std::nullopt)
        profiles.push_back (std::make_unique<CabinEQValueTree> (apvts, profileName));
    
    std::cout << "Added profile named " << profileName << std::endl;
    std::cout << "Current profile list: " << std::endl;
    for (const auto& profile : profiles)
        std::cout << "Profile: " << profile->getName() << std::endl;
}

void CabinEQValueTreeManager::removeProfile (juce::String profileName)
{
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            profiles.erase (profiles.begin() + i);
}

void CabinEQValueTreeManager::initProfiles()
{
    for (const auto& profile : profiles)
        profile->initValueTreeFromAPVTS();
}

const std::vector<juce::String> CabinEQValueTreeManager::getProfileNames() const
{
    std::vector<juce::String> profileNames;
    for (const auto& profile : profiles)
        profileNames.push_back (profile->getName());
    return profileNames;
}

std::optional<std::reference_wrapper<CabinEQValueTree>> CabinEQValueTreeManager::getProfileNamed (juce::String profileName) const
{
    std::cout << "getting profile named: " << profileName << std::endl;
    for (int i = 0; i < profiles.size(); ++i)
        if (profiles[i]->getName() == profileName)
            return std::ref (*profiles[i]);
    
    std::cout << "couldn't find any profile named " << profileName << std::endl;
    
    return std::nullopt;
}
