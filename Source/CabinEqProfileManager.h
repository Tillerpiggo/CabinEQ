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

/// The list of profiles, which are the <Profile> children of the plugin's state, in order,
/// and which one is selected. Holds nothing itself, so it's always in step with the state.
class CabinEqProfileManager
{
public:
    CabinEqProfileManager (juce::AudioProcessorValueTreeState& apvts, juce::UndoManager& undoManager);

    /// Brings state saved by an older CabinEQ up to date, before it's loaded. Saves a copy of
    /// the old state in getBackupFolder() first, since older versions can't read the new format.
    static void migrateState (juce::ValueTree& state);
    static juce::File getBackupFolder(); // ~/Library/Application Support/CabinEQ/Backups on a Mac
    static inline bool shouldBackUpOldState = true; // the tests turn this off

    /// Makes sure there's at least one profile, and that one of them is selected (fallback if it
    /// exists, otherwise the first). Not undoable.
    void ensureValidState (const juce::String& fallback = {});

    /// The selected profile's bands in a state that isn't loaded, or the first profile's.
    static BandProfile getSelectedBandProfile (const juce::ValueTree& state);

    std::vector<juce::String> getProfileNames() const;
    std::optional<CabinEqProfile> getProfileNamed (const juce::String& profileName) const;
    CabinEqProfile getSelectedProfile() const; // may be invalid if ensureValidState() hasn't run

    juce::String getSelectedProfileName() const;
    void setSelectedProfileName (const juce::String& profileName);

    /// These return the profile they made; names are made unique if they're taken.
    CabinEqProfile addProfile (const juce::String& profileName, const BandProfile& bandProfile = {});
    CabinEqProfile duplicateProfile (const juce::String& profileName);
    void removeProfile (const juce::String& profileName);
    void renameProfile (const juce::String& profileName, const juce::String& newProfileName);

    juce::String makeUniqueName (const juce::String& wantedName, const juce::String& ignoring = {}) const;
    juce::String getProfileNameContaining (const juce::ValueTree& tree) const; // or empty

    static inline const juce::String defaultProfileName { "My Headphone Profile" };
    static inline const juce::Identifier idSelectedProfile { "lastSelectedProfileId" };
    static inline const juce::Identifier idStateVersion { "stateVersion" };
    static constexpr int stateVersion = 2;

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::UndoManager& undoManager;
};
