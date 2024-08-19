/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQValueTree.h"

CabinEQValueTree::CabinEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : apvts (apvts), idProfile (identifier), idCurvePt ("CurvePt"), idId ("id"), idFreq ("freq"), idVal ("val"), idAmplTree ("AmplTree"), idPanTree ("PanTree")
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

void CabinEQValueTree::addAmplPt (const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    int id = getNextIdForCurvePtTree (amplPtTree);
    addCurvePtToTree (id, freq, ampl, amplPtTree);
}

void CabinEQValueTree::addPanPt (const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    int id = getNextIdForCurvePtTree (panPtTree);
    addCurvePtToTree (id, freq, pan, panPtTree);
}

void CabinEQValueTree::removeAmplPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    
    juce::ValueTree nodeToRemove = amplPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplPtTree.removeChild (nodeToRemove, nullptr);
}

void CabinEQValueTree::removePanPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    
    juce::ValueTree nodeToRemove = panPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        panPtTree.removeChild (nodeToRemove, nullptr);
}

void CabinEQValueTree::updateAmplPt (const int id, const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    updateCurvePtInTree (id, freq, ampl, amplPtTree);
}

void CabinEQValueTree::updatePanPt (const int id, const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    updateCurvePtInTree (id, freq, pan, panPtTree);
}

void CabinEQValueTree::resetNodes()
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
//    curve.updateWithEQNodes (getEQNodes());
}

void CabinEQValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithName (idProfile);

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        juce::ValueTree amplPtTree (idAmplTree);
        juce::ValueTree panPtTree (idPanTree);
        valueTree.addChild (amplPtTree, 0, nullptr);
        valueTree.addChild (panPtTree, 1, nullptr);
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    
//    curve.updateWithEQNodes (getEQNodes());
    hasBeenInitialized = true;
}

const juce::String CabinEQValueTree::getName() const
{
    return idProfile.toString();
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
    valueTree.appendChild (curvePt, nullptr);
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
        id = std::max ((int) curvePtTree.getProperty (idId), id);
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
        juce::Identifier idId ("id");
        juce::Identifier idFrequency ("frequency");
        juce::Identifier idAmplitude ("amplitude");
        if (valueTree.getNumChildren() > 0)
        {
            for (const auto& eqNode : valueTree)
            {
                float id = eqNode.getProperty (idId);
                float freq = eqNode.getProperty (idFrequency);
                float ampl = eqNode.getProperty (idAmplitude);
                
                std::cout << "EQNode (id: " << id << ", freq: " << freq << ", ampl: " << ampl << ")" << std::endl;
            }
        }
    }
    else
    {
        std::cout << "NO_TREE" << std::endl;
    }
    std::cout << std::endl;
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
