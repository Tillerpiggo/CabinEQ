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

/// A handle to one profile's ValueTree, which is where its bands and preamp are saved:
///
///     <Profile ProfileName="..." ProfileVolume="0.0">
///       <Bands>
///         <Band id="0" freq="1000" ampl="3" bandwidth="1" bandtype="0" shape="0" enabled="1"/>
///       </Bands>
///     </Profile>
///
/// Copies share the same tree. Edits go through the UndoManager, if there is one.
class CabinEqProfile
{
public:
    CabinEqProfile (juce::ValueTree profileTree, juce::UndoManager* undoManager);

    static juce::ValueTree createTree (const juce::String& name, const BandProfile& bandProfile = {});

    /// Converts a profile saved before bands were flattened, where bands lived in <AmplTree> under
    /// several <MultiBandStep>s. Bands from disabled steps are kept, but disabled. Also drops the
    /// properties the calibration and free trial used. Returns true if anything changed.
    static bool migrate (juce::ValueTree profileTree);

    bool isValid() const;
    juce::ValueTree getTree() const;

    juce::String getName() const;
    float getVolume() const; // preamp, in dB
    BandProfile getBandProfile() const;
    std::optional<Band> getBand (int id) const;
    int getNumBands() const;

    /// Adds the band with a new id, and returns the id, or -1 if the profile is full.
    int addBand (const Band& band);
    void updateBand (const Band& band); // matches on id
    void removeBand (int id);
    void setBands (const std::vector<Band>& bands); // replaces every band, giving them new ids

    void setVolume (float volume);
    void renameTo (const juce::String& newName);

    static inline const juce::Identifier idProfile { "Profile" };
    static inline const juce::Identifier idProfileName { "ProfileName" };
    static inline const juce::Identifier idProfileVolume { "ProfileVolume" };
    static inline const juce::Identifier idBands { "Bands" };
    static inline const juce::Identifier idBand { "Band" };
    static inline const juce::Identifier idId { "id" };
    static inline const juce::Identifier idFreq { "freq" };
    static inline const juce::Identifier idAmpl { "ampl" };
    static inline const juce::Identifier idBandwidth { "bandwidth" };
    static inline const juce::Identifier idBandType { "bandtype" };
    static inline const juce::Identifier idShape { "shape" };
    static inline const juce::Identifier idEnabled { "enabled" };

private:
    static Band bandFromTree (const juce::ValueTree& bandTree);
    static juce::ValueTree treeFromBand (const Band& band);
    juce::ValueTree getBandsTree();
    int getNextBandId() const;

    juce::ValueTree tree;
    juce::UndoManager* undoManager;
};
