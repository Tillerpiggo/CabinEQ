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
    
    const std::vector<CurvePt> getLeftAmplPts() const; // constructs curve pts matching the ones in memory
    const std::vector<CurvePt> getRightAmplPts() const; // constructs curve pts matching the ones in memory
    const std::optional<CurvePt> getLeftAmplPtWithId (const int id) const;
    const std::optional<CurvePt> getRightAmplPtWithId (const int id) const;
    const std::optional<CurvePt> getSpatialPtWithId (const int id) const;
    Curve& getLeftAmplCurve();
    Curve& getRightAmplCurve();
    Curve& getSpatialCurve();
    
    int addLeftAmplPt (const float freq, const float ampl);
    int addRightAmplPt (const float freq, const float ampl);
    int addSpatialPt (const float freq, const float ampl);
    void removeLeftAmplPt (const int id);
    void removeRightAmplPt (const int id);
    void removeSpatialPt (const int id);
    void updateLeftAmplPt (const int id, const float freq, const float ampl);
    void updateRightAmplPt (const int id, const float freq, const float ampl);
    void updateSpatialPt (const int id, const float freq, const float ampl);
    
    void resetNodes(); // makes this value tree store the given set points
    
    void initValueTreeFromAPVTS(); // sets value tree to match the one in apvts
    const juce::String getName() const;
    
    void copyFrom (CabinEqValueTree& other);
    
private:
    void addCurvePtToTree (int id, float freq, float val, juce::ValueTree curvePtTree);
    void updateCurvePtInTree (int id, float freq, float val, juce::ValueTree curvePtTree);
    int getNextIdForCurvePtTree (juce::ValueTree curvePtTree);
    void resetAPVTS (juce::AudioProcessorValueTreeState& apvts);
    void printValueTree (juce::ValueTree valueTree) const;
    void updateCurves(); // updates both curves to match the current state of the value tree
    void updateLeftAmplCurve();
    void updateRightAmplCurve();
    void updateSpatialCurve();
    std::vector<CurvePt> getCurvePtsForValueTree (juce::ValueTree valueTree) const;
    
    juce::AudioProcessorValueTreeState& apvts;
    
    juce::Identifier idProfile { "Profile" };
    juce::Identifier idProfileName { "ProfileName" };
    juce::Identifier idCurvePt { "CurvePt" };
    juce::Identifier idId { "id" };
    juce::Identifier idFreq { "freq" };
    juce::Identifier idVal { "val" };
    juce::Identifier idLeftAmplTree { "LeftAmplTree" };
    juce::Identifier idRightAmplTree { "RightAmplTree" };
    juce::Identifier idSpatialTree { "SpatialTree" };
    juce::ValueTree valueTree;
    juce::String profileName;
    
    Curve leftAmplCurve;
    Curve rightAmplCurve;
    Curve spatialCurve;
    bool hasBeenInitialized = false;
};
