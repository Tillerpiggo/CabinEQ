/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEqValueTree.h"

CabinEqValueTree::CabinEqValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String identifier)
    : apvts (apvts), profileName (identifier)
{}

const std::vector<CurvePt> CabinEqValueTree::getLeftAmplPts() const
{
    std::vector<CurvePt> amplitudes;
    if (! valueTree.isValid())
        return amplitudes;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idLeftAmplTree));
}

const std::vector<CurvePt> CabinEqValueTree::getRightAmplPts() const
{
    std::vector<CurvePt> pans;
    if (! valueTree.isValid())
        return pans;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idRightAmplTree));
}

const std::optional<CurvePt> CabinEqValueTree::getLeftAmplPtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto amplPtTree = valueTree.getChildWithName (idLeftAmplTree);
    if (! amplPtTree.isValid())
        return std::nullopt;
    
    auto amplPt = amplPtTree.getChildWithProperty (idId, id);
    if (amplPt.isValid())
    {
        return CurvePt (amplPt.getProperty (idId),
                       amplPt.getProperty (idFreq),
                       amplPt.getProperty (idVal));
    }
    else
    {
        return std::nullopt;
    }
}

const std::optional<CurvePt> CabinEqValueTree::getRightAmplPtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto panPtTree = valueTree.getChildWithName (idRightAmplTree);
    if (! panPtTree.isValid())
        return std::nullopt;
    
    auto panPt = panPtTree.getChildWithProperty (idId, id);
    if (panPt.isValid())
    {
        return CurvePt (panPt.getProperty (idId),
                        panPt.getProperty (idFreq),
                        panPt.getProperty (idVal));
    }
    else
    {
        return std::nullopt;
    }
}

const std::optional<CurvePt> CabinEqValueTree::getSpatialPtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto spatialTree = valueTree.getChildWithName (idSpatialTree);
    if (! spatialTree.isValid())
        return std::nullopt;
    
    auto spatialPt = spatialTree.getChildWithProperty (idId, id);
    if (spatialPt.isValid())
    {
        return CurvePt (spatialPt.getProperty (idId),
                        spatialPt.getProperty (idFreq),
                        spatialPt.getProperty (idVal));
    }
    else
    {
        return std::nullopt;
    }
}

Curve& CabinEqValueTree::getLeftAmplCurve()
{
    return leftAmplCurve;
}

Curve& CabinEqValueTree::getRightAmplCurve()
{
    return rightAmplCurve;
}

Curve& CabinEqValueTree::getSpatialCurve()
{
    return spatialCurve;
}

int CabinEqValueTree::addLeftAmplPt (const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idLeftAmplTree);
    int id = getNextIdForCurvePtTree (amplPtTree);
    addCurvePtToTree (id, freq, ampl, amplPtTree);
    
    updateLeftAmplCurve();
    
    return id;
}

int CabinEqValueTree::addRightAmplPt (const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idRightAmplTree);
    int id = getNextIdForCurvePtTree (panPtTree);
    addCurvePtToTree (id, freq, pan, panPtTree);
    
    updateRightAmplCurve();
    
    return id;
}

int CabinEqValueTree::addSpatialPt (const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto spatialTree = valueTree.getChildWithName (idSpatialTree);
    int id = getNextIdForCurvePtTree (spatialTree);
    addCurvePtToTree (id, freq, pan, spatialTree);
    
    updateSpatialCurve();
    
    return id;
}

void CabinEqValueTree::removeLeftAmplPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idLeftAmplTree);
    
    juce::ValueTree nodeToRemove = amplPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplPtTree.removeChild (nodeToRemove, nullptr);
    
    updateLeftAmplCurve();
}

void CabinEqValueTree::removeRightAmplPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idRightAmplTree);
    
    juce::ValueTree nodeToRemove = panPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        panPtTree.removeChild (nodeToRemove, nullptr);
    
    updateRightAmplCurve();
}

void CabinEqValueTree::removeSpatialPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto spatialTree = valueTree.getChildWithName (idSpatialTree);
    
    juce::ValueTree nodeToRemove = spatialTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        spatialTree.removeChild (nodeToRemove, nullptr);
    
    updateSpatialCurve();
}


void CabinEqValueTree::updateLeftAmplPt (const int id, const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idLeftAmplTree);
    updateCurvePtInTree (id, freq, ampl, amplPtTree);
    
    updateLeftAmplCurve();
}

void CabinEqValueTree::updateRightAmplPt (const int id, const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idRightAmplTree);
    updateCurvePtInTree (id, freq, pan, panPtTree);
    
    updateRightAmplCurve();
}

void CabinEqValueTree::updateSpatialPt (const int id, const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto spatialTree = valueTree.getChildWithName (idSpatialTree);
    updateCurvePtInTree (id, freq, ampl, spatialTree);
    
    updateSpatialCurve();
}

void CabinEqValueTree::resetNodes()
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    updateCurves();
}

void CabinEqValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithProperty (idProfileName, profileName);

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        valueTree.setProperty (idProfileName, profileName, nullptr);
        auto leftAmplPtTree = juce::ValueTree (idLeftAmplTree);
        auto rightAmplPtTree = juce::ValueTree (idRightAmplTree);
        auto spatialTree = juce::ValueTree (idSpatialTree);
        valueTree.addChild (leftAmplPtTree, 0, nullptr);
        valueTree.addChild (rightAmplPtTree, 1, nullptr);
        valueTree.addChild (spatialTree, 2, nullptr);
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    else
    {
        profileName = valueTree.getProperty (idProfileName);
    }
    
    updateCurves();
    hasBeenInitialized = true;
}

const juce::String CabinEqValueTree::getName() const
{
    return profileName;
}

void CabinEqValueTree::copyFrom (CabinEqValueTree& other)
{
    initValueTreeFromAPVTS();
    valueTree.copyPropertiesAndChildrenFrom (other.valueTree, nullptr);
}

void CabinEqValueTree::addCurvePtToTree (int id, float freq, float val, juce::ValueTree curvePtTree)
{
    juce::ValueTree curvePt (idCurvePt);
    curvePt.setProperty (idId, id, nullptr);
    curvePt.setProperty (idFreq, freq, nullptr);
    curvePt.setProperty (idVal, val, nullptr);
    curvePtTree.appendChild (curvePt, nullptr);
}

void CabinEqValueTree::updateCurvePtInTree (int id, float freq, float val, juce::ValueTree curvePtTree)
{
    juce::ValueTree curvePtToModify = curvePtTree.getChildWithProperty (idId, id);
    
    if (curvePtToModify.isValid())
    {
        curvePtToModify.setProperty (idFreq, freq, nullptr);
        curvePtToModify.setProperty (idVal, val, nullptr);
    }
}

int CabinEqValueTree::getNextIdForCurvePtTree (juce::ValueTree curvePtTree)
{
    // Assume tree is valid; otherwise this should crash
    int id = -1;
    for (const auto& curvePt : curvePtTree)
    {
        id = std::max ((int) curvePt.getProperty (idId), id);
    }
    id++;
    
    return id;
}

void CabinEqValueTree::resetAPVTS (juce::AudioProcessorValueTreeState& apvts)
{
    apvts.state.removeAllChildren (nullptr);
    apvts.state.removeAllProperties (nullptr);
}

void CabinEqValueTree::printValueTree (juce::ValueTree valueTree) const
{
    if (valueTree.isValid())
    {
        std::cout << "ClearEQValueTree (numNodes: " << valueTree.getNumChildren() << ")" << std::endl;
        if (valueTree.getNumChildren() > 0)
        {
            for (const auto& eqNode : valueTree)
            {
                float id = eqNode.getProperty (idId);
                float freq = eqNode.getProperty (idFreq);
                float ampl = eqNode.getProperty (idVal);
                
                std::cout << "EQNode (id: " << id << ", freq: " << freq << ", val: " << ampl << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "NO_TREE" << std::endl;
    }
    std::cout << std::endl;
}

void CabinEqValueTree::updateCurves()
{
    updateLeftAmplCurve();
    updateRightAmplCurve();
}

void CabinEqValueTree::updateLeftAmplCurve()
{
    leftAmplCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idLeftAmplTree)));
}

void CabinEqValueTree::updateRightAmplCurve()
{
    rightAmplCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idRightAmplTree)));
}

void CabinEqValueTree::updateSpatialCurve()
{
    spatialCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idSpatialTree)));
}

std::vector<CurvePt> CabinEqValueTree::getCurvePtsForValueTree (juce::ValueTree curvePtValueTree) const
{
    std::vector<CurvePt> curvePts;
    
    if (! curvePtValueTree.isValid())
        return curvePts;
    
    for (const auto& curvePt : curvePtValueTree)
    {
        int id = curvePt.getProperty (idId);
        float freq = curvePt.getProperty (idFreq);
        float val = curvePt.getProperty (idVal);
        curvePts.emplace_back (id, freq, val);
    }
    
    return curvePts;
}
