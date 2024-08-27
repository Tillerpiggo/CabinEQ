/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQValueTree.h"

CabinEQValueTree::CabinEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String identifier)
    : apvts (apvts), idProfile ("Profile"), idProfileName ("ProfileName"), idCurvePt ("CurvePt"), idId ("id"), idFreq ("freq"), idAmplTree ("AmplTree"), idPanTree ("PanTree"), idVal ("val"), profileName (identifier)
{}

const std::vector<CurvePt> CabinEQValueTree::getAmplPts() const
{
    std::vector<CurvePt> amplitudes;
    if (! valueTree.isValid())
        return amplitudes;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idAmplTree));
}

const std::vector<CurvePt> CabinEQValueTree::getPanPts() const
{
    std::vector<CurvePt> pans;
    if (! valueTree.isValid())
        return pans;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idPanTree));
}

const std::optional<CurvePt> CabinEQValueTree::getAmplPtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
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

const std::optional<CurvePt> CabinEQValueTree::getPanPtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
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

Curve& CabinEQValueTree::getAmplCurve()
{
    return amplCurve;
}

Curve& CabinEQValueTree::getPanCurve()
{
    return panCurve;
}

int CabinEQValueTree::addAmplPt (const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    int id = getNextIdForCurvePtTree (amplPtTree);
    addCurvePtToTree (id, freq, ampl, amplPtTree);
    
    updateAmplCurve();
    
    return id;
}

int CabinEQValueTree::addPanPt (const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    int id = getNextIdForCurvePtTree (panPtTree);
    addCurvePtToTree (id, freq, pan, panPtTree);
    
//    printValueTree (panPtTree);
    
    updatePanCurve();
    
    return id;
}

void CabinEQValueTree::removeAmplPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    
    juce::ValueTree nodeToRemove = amplPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplPtTree.removeChild (nodeToRemove, nullptr);
    
    updateAmplCurve();
}

void CabinEQValueTree::removePanPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    
    juce::ValueTree nodeToRemove = panPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        panPtTree.removeChild (nodeToRemove, nullptr);
    
    updatePanCurve();
}

void CabinEQValueTree::updateAmplPt (const int id, const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    updateCurvePtInTree (id, freq, ampl, amplPtTree);
    
    updateAmplCurve();
}

void CabinEQValueTree::updatePanPt (const int id, const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    updateCurvePtInTree (id, freq, pan, panPtTree);
    
    updatePanCurve();
}

void CabinEQValueTree::resetNodes()
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    updateCurves();
}

void CabinEQValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithProperty (idProfileName, profileName);

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        valueTree.setProperty (idProfileName, profileName, nullptr);
        auto amplPtTree = juce::ValueTree (idAmplTree);
        auto panPtTree = juce::ValueTree (idPanTree);
        valueTree.addChild (amplPtTree, 0, nullptr);
        valueTree.addChild (panPtTree, 1, nullptr);
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    else
    {
        profileName = valueTree.getProperty (idProfileName);
    }
    
    updateCurves();
    hasBeenInitialized = true;
}

const juce::String CabinEQValueTree::getName() const
{
    return profileName;
}

void CabinEQValueTree::copyFrom (CabinEQValueTree& other)
{
    initValueTreeFromAPVTS();
    valueTree.copyPropertiesAndChildrenFrom (other.valueTree, nullptr);
}

void CabinEQValueTree::addCurvePtToTree (int id, float freq, float val, juce::ValueTree curvePtTree)
{
    juce::ValueTree curvePt (idCurvePt);
    curvePt.setProperty (idId, id, nullptr);
    curvePt.setProperty (idFreq, freq, nullptr);
    curvePt.setProperty (idVal, val, nullptr);
    curvePtTree.appendChild (curvePt, nullptr);
}

void CabinEQValueTree::updateCurvePtInTree (int id, float freq, float val, juce::ValueTree curvePtTree)
{
    juce::ValueTree curvePtToModify = curvePtTree.getChildWithProperty (idId, id);
    
    if (curvePtToModify.isValid())
    {
        curvePtToModify.setProperty (idFreq, freq, nullptr);
        curvePtToModify.setProperty (idVal, val, nullptr);
    }
}

int CabinEQValueTree::getNextIdForCurvePtTree (juce::ValueTree curvePtTree)
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

void CabinEQValueTree::resetAPVTS (juce::AudioProcessorValueTreeState& apvts)
{
    apvts.state.removeAllChildren (nullptr);
    apvts.state.removeAllProperties (nullptr);
}

void CabinEQValueTree::printValueTree (juce::ValueTree valueTree) const
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

void CabinEQValueTree::updateCurves()
{
    updateAmplCurve();
    updatePanCurve();
}

void CabinEQValueTree::updateAmplCurve()
{
    amplCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idAmplTree)));
}

void CabinEQValueTree::updatePanCurve()
{
    panCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idPanTree)));
}

std::vector<CurvePt> CabinEQValueTree::getCurvePtsForValueTree (juce::ValueTree curvePtValueTree) const
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
