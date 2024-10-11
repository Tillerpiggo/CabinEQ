/*
  ==============================================================================

    CabinEqProfileManager.cpp
    Created: 10 Oct 2024 7:51:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqProfileManager.h"

CabinEqProfileManager::CabinEqProfileManager (juce::AudioProcessorValueTreeState& apts)
    : apvts (apvts)
{}

void CabinEqProfileManager::addProfile (juce::String profileName)
{
    // Create the profile
    if (getProfileNamed (profileName) == std::nullopt)
    {
        auto newProfile = std::make_unique<CabinEqProfile> (apvts, profileName);
        
        
    }
}
