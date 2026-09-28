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

/// A handle to one profile's ValueTree, which is where its EQ and preamp are saved:
///
///     <Profile ProfileName="..." ProfileVolume="0.0" mode="bands">
///       <Bands>
///         <Band id="0" freq="1000" ampl="3" bandwidth="1" bandtype="0" shape="0" enabled="1"/>
///       </Bands>
///       <Curve>
///         <Point id="0" freq="100" gain="4"/>
///       </Curve>
///       <CurveLeft> ... </CurveLeft>
///       <CurveRight> ... </CurveRight>
///     </Profile>
///
/// "mode" says which of the two is playing; both are kept, so switching loses nothing. <Curve> is
/// what both ears get. With curveSplit="1", <CurveLeft> and <CurveRight> are each ear's tweak on top.
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

    // Curve mode. `layer` is a BandProfile::Layer: the shared curve, or (when split) an ear's tweak.
    BandProfile::Mode getMode() const;
    void setMode (BandProfile::Mode mode);
    std::optional<CurvePoint> getPoint (int id, int layer = BandProfile::both) const;
    int getNumPoints (int layer = BandProfile::both) const;
    int addPoint (const CurvePoint& point, int layer = BandProfile::both); // returns its new id, or -1 if the curve is full
    void updatePoint (const CurvePoint& point, int layer = BandProfile::both); // matches on id
    void removePoint (int id, int layer = BandProfile::both);
    void setPoints (const std::vector<CurvePoint>& points, int layer = BandProfile::both); // replaces them all, giving them new ids

    /// Splitting starts both ears with no tweaks; joining drops the tweaks and keeps the shared curve.
    bool isCurveSplit() const;
    void setCurveSplit (bool shouldSplit);
    static constexpr int maxPoints = 64;

    static inline const juce::Identifier idProfile { "Profile" };
    static inline const juce::Identifier idProfileName { "ProfileName" };
    static inline const juce::Identifier idProfileVolume { "ProfileVolume" };
    static inline const juce::Identifier idBands { "Bands" };
    static inline const juce::Identifier idMode { "mode" };
    static inline const juce::Identifier idCurve { "Curve" };
    static inline const juce::Identifier idCurveLeft { "CurveLeft" };
    static inline const juce::Identifier idCurveRight { "CurveRight" };
    static inline const juce::Identifier idCurveSplit { "curveSplit" };
    static inline const juce::Identifier idPoint { "Point" };
    static inline const juce::Identifier idGain { "gain" };
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
    juce::ValueTree getCurveTree (int layer);
    juce::Identifier curveNameFor (int layer) const;
    static juce::ValueTree treeFromPoints (const juce::Identifier& name, const std::vector<CurvePoint>& points);
    static std::vector<CurvePoint> pointsFromTree (const juce::ValueTree& curveTree);
    int getNextBandId() const;

    juce::ValueTree tree;
    juce::UndoManager* undoManager;
};
