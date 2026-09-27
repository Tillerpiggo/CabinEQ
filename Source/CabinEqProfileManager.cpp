/*
  ==============================================================================

    CabinEqProfileManager.cpp
    Created: 10 Oct 2024 7:51:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqProfileManager.h"

namespace
{
    // Properties older versions kept on the root of the state
    const juce::Identifier idMasterVolume { "masterVolumeId" };
    const juce::Identifier idHasLicense { "hasLicenseId" };
}

CabinEqProfileManager::CabinEqProfileManager (juce::AudioProcessorValueTreeState& apvts, juce::UndoManager& undoManager)
    : apvts (apvts), undoManager (undoManager)
{}

void CabinEqProfileManager::migrateState (juce::ValueTree& state)
{
    if ((int) state.getProperty (idStateVersion, 1) >= stateVersion)
        return;

    // Older CabinEQs can't read the new format, so keep a copy to go back to
    if (shouldBackUpOldState && state.getChildWithName (CabinEqProfile::idProfile).isValid())
    {
        auto folder = getBackupFolder();
        if (folder.createDirectory())
        {
            auto file = folder.getNonexistentChildFile ("CabinEQ state before update " + juce::Time::getCurrentTime().formatted ("%Y-%m-%d %H.%M.%S"), ".xml");
            file.replaceWithText (state.toXmlString());
        }
    }

    // The master volume slider went away, so fold it into each profile's preamp so nothing sounds different
    const float masterVolume = state.getProperty (idMasterVolume, 0.0f);

    for (auto child : state)
    {
        if (! child.hasType (CabinEqProfile::idProfile))
            continue;

        CabinEqProfile::migrate (child);
        if (masterVolume != 0.0f)
            child.setProperty (CabinEqProfile::idProfileVolume,
                               juce::jlimit (-30.0f, 30.0f, (float) child.getProperty (CabinEqProfile::idProfileVolume, 0.0f) + masterVolume),
                               nullptr);
    }

    state.removeProperty (idMasterVolume, nullptr);
    state.removeProperty (idHasLicense, nullptr);

    // Drop the old placeholder parameter, and keep auto gain off so existing profiles sound the same
    for (int i = state.getNumChildren(); --i >= 0;)
        if (state.getChild (i).hasType ("PARAM") && state.getChild (i).getProperty ("id") == "dummyParam")
            state.removeChild (i, nullptr);

    if (! state.getChildWithProperty ("id", "autoGain").isValid())
    {
        juce::ValueTree autoGain ("PARAM");
        autoGain.setProperty ("id", "autoGain", nullptr);
        autoGain.setProperty ("value", 0.0f, nullptr);
        state.appendChild (autoGain, nullptr);
    }
    state.setProperty (idStateVersion, stateVersion, nullptr);
}

juce::File CabinEqProfileManager::getBackupFolder()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Application Support").getChildFile ("CabinEQ").getChildFile ("Backups");
}

void CabinEqProfileManager::ensureValidState()
{
    auto& state = apvts.state;
    state.setProperty (idStateVersion, stateVersion, nullptr);

    if (getProfileNames().empty())
        state.appendChild (CabinEqProfile::createTree (defaultProfileName), nullptr);

    if (! getProfileNamed (getSelectedProfileName()).has_value())
        setSelectedProfileName (getProfileNames().front());
}

std::vector<juce::String> CabinEqProfileManager::getProfileNames() const
{
    std::vector<juce::String> names;
    for (const auto& child : apvts.state)
        if (child.hasType (CabinEqProfile::idProfile))
            names.push_back (child.getProperty (CabinEqProfile::idProfileName).toString());
    return names;
}

std::optional<CabinEqProfile> CabinEqProfileManager::getProfileNamed (const juce::String& profileName) const
{
    for (const auto& child : apvts.state)
        if (child.hasType (CabinEqProfile::idProfile) && child.getProperty (CabinEqProfile::idProfileName).toString() == profileName)
            return CabinEqProfile (child, &undoManager);
    return std::nullopt;
}

CabinEqProfile CabinEqProfileManager::getSelectedProfile() const
{
    return getProfileNamed (getSelectedProfileName()).value_or (CabinEqProfile ({}, &undoManager));
}

juce::String CabinEqProfileManager::getSelectedProfileName() const
{
    return apvts.state.getProperty (idSelectedProfile).toString();
}

void CabinEqProfileManager::setSelectedProfileName (const juce::String& profileName)
{
    // Which profile you're looking at isn't something to undo
    apvts.state.setProperty (idSelectedProfile, profileName, nullptr);
}

CabinEqProfile CabinEqProfileManager::addProfile (const juce::String& profileName, const BandProfile& bandProfile)
{
    auto profileTree = CabinEqProfile::createTree (makeUniqueName (profileName), bandProfile);
    apvts.state.appendChild (profileTree, &undoManager);
    return CabinEqProfile (profileTree, &undoManager);
}

CabinEqProfile CabinEqProfileManager::duplicateProfile (const juce::String& profileName)
{
    auto original = getProfileNamed (profileName);
    if (! original.has_value())
        return CabinEqProfile ({}, &undoManager);

    auto copy = original->getTree().createCopy();
    copy.setProperty (CabinEqProfile::idProfileName, makeUniqueName (profileName + " copy"), nullptr);

    // Put the copy right after the original
    const int index = apvts.state.indexOf (original->getTree());
    apvts.state.addChild (copy, index + 1, &undoManager);
    return CabinEqProfile (copy, &undoManager);
}

void CabinEqProfileManager::removeProfile (const juce::String& profileName)
{
    if (auto profile = getProfileNamed (profileName))
        apvts.state.removeChild (profile->getTree(), &undoManager);
}

void CabinEqProfileManager::renameProfile (const juce::String& profileName, const juce::String& newProfileName)
{
    auto profile = getProfileNamed (profileName);
    auto trimmed = newProfileName.trim();
    if (! profile.has_value() || trimmed.isEmpty() || trimmed == profileName)
        return;

    const bool wasSelected = getSelectedProfileName() == profileName;
    profile->renameTo (makeUniqueName (trimmed, profileName));
    if (wasSelected)
        setSelectedProfileName (profile->getName());
}

juce::String CabinEqProfileManager::makeUniqueName (const juce::String& wantedName, const juce::String& ignoring) const
{
    auto base = wantedName.trim();
    if (base.isEmpty())
        base = "Profile";

    auto names = getProfileNames();
    auto isTaken = [&] (const juce::String& name)
    {
        return name != ignoring && std::find (names.begin(), names.end(), name) != names.end();
    };

    auto name = base;
    for (int number = 2; isTaken (name); ++number)
        name = base + " " + juce::String (number);
    return name;
}

juce::String CabinEqProfileManager::getProfileNameContaining (const juce::ValueTree& tree) const
{
    for (auto node = tree; node.isValid(); node = node.getParent())
        if (node.hasType (CabinEqProfile::idProfile))
            return node.getProperty (CabinEqProfile::idProfileName).toString();
    return {};
}
