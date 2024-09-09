/*
  ==============================================================================

    CurveValueTree.h
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CurvePt.h"
#include "Curve.h"

/// This class wraps and provides helper methods on a ValueTree to persist the set points of a single profile. It should be used to initially load and save
/// this data, rather than to actively manage it.
class CabinEqValueTree
{
public:
    CabinEqValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String identifier);
    
    const std::vector<CurvePt> getAmplPts() const; // constructs curve pts matching the ones in memory
    const std::vector<CurvePt> getPanPts() const; // constructs curve pts matching the ones in memory
    const std::vector<CurvePt> getPhasePts() const; // constructs curve pts matching the ones in memory
    const std::optional<CurvePt> getAmplPtWithId (const int id) const;
    const std::optional<CurvePt> getPanPtWithId (const int id) const;
    const std::optional<CurvePt> getPhasePtWithId (const int id) const;
    Curve& getAmplCurve();
    Curve& getPanCurve();
    Curve& getPhaseCurve();
    
    int addAmplPt (const float freq, const float ampl);
    int addPanPt (const float freq, const float pan);
    int addPhasePt (const float freq, const float phase);
    void removeAmplPt (const int id);
    void removePanPt (const int id);
    void removePhasePt (const int id);
    void updateAmplPt (const int id, const float freq, const float ampl);
    void updatePanPt (const int id, const float freq, const float pan);
    void updatePhasePt (const int id, const float freq, const float phase);
    
    void resetNodes(); // makes this value tree store the given set points
    
    void initValueTreeFromAPVTS(); // sets value tree to match the one in apvts
    const juce::String getName() const;
    
    void copyFrom (CabinEqValueTree& other);
    void renameTo (juce::String newName);
    
private:
    void addCurvePtToTree (int id, float freq, float val, juce::ValueTree curvePtTree);
    void updateCurvePtInTree (int id, float freq, float val, juce::ValueTree curvePtTree);
    int getNextIdForCurvePtTree (juce::ValueTree curvePtTree);
    void resetAPVTS (juce::AudioProcessorValueTreeState& apvts);
    void printValueTree (juce::ValueTree valueTree) const;
    void updateCurves(); // updates both curves to match the current state of the value tree
    void updateAmplCurve();
    void updatePanCurve();
    void updatePhaseCurve();
    std::vector<CurvePt> getCurvePtsForValueTree (juce::ValueTree valueTree) const;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::Identifier idProfile { "Profile" };
    juce::Identifier idProfileName { "ProfileName" };
    juce::Identifier idCurvePt { "CurvePt" };
    juce::Identifier idId { "id" };
    juce::Identifier idFreq { "freq" };
    juce::Identifier idVal { "val" };
    juce::Identifier idAmplTree { "AmplTree" };
    juce::Identifier idPanTree { "PanTree" };
    juce::Identifier idPhaseTree { "PhaseTree" };
    juce::ValueTree valueTree;
    juce::String profileName;
    
    Curve amplCurve;
    Curve panCurve;
    Curve phaseCurve;
    bool hasBeenInitialized = false;
};
