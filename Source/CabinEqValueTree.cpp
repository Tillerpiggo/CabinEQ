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

const std::vector<CurvePt> CabinEqValueTree::getAmplPts() const
{
    std::vector<CurvePt> amplitudes;
    if (! valueTree.isValid())
        return amplitudes;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idAmplTree));
}

const std::vector<CurvePt> CabinEqValueTree::getPanPts() const
{
    std::vector<CurvePt> pans;
    if (! valueTree.isValid())
        return pans;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idPanTree));
}

const std::vector<CurvePt> CabinEqValueTree::getPhasePts() const
{
    std::vector<CurvePt> phases;
    if (! valueTree.isValid())
        return phases;
    
    return getCurvePtsForValueTree (valueTree.getChildWithName (idPhaseTree));
}

const std::optional<CurvePt> CabinEqValueTree::getAmplPtWithId (const int id) const
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

const std::optional<CurvePt> CabinEqValueTree::getPanPtWithId (const int id) const
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

const std::optional<CurvePt> CabinEqValueTree::getPhasePtWithId (const int id) const
{
    if (! valueTree.isValid())
        return std::nullopt;
    
    auto phasePtTree = valueTree.getChildWithName (idPhaseTree);
    if (! phasePtTree.isValid())
        return std::nullopt;
    
    auto phasePt = phasePtTree.getChildWithProperty (idId, id);
    if (phasePt.isValid())
    {
        return CurvePt (phasePt.getProperty (idId),
                        phasePt.getProperty (idFreq),
                        phasePt.getProperty (idVal));
    }
    else
    {
        return std::nullopt;
    }
}

Curve& CabinEqValueTree::getAmplCurve()
{
    updateAmplCurve();
    return amplCurve;
}

Curve& CabinEqValueTree::getPanCurve()
{
    return panCurve;
}

Curve& CabinEqValueTree::getPhaseCurve()
{
    return phaseCurve;
}


int CabinEqValueTree::addAmplPt (const float freq, const float ampl)
{
    std::cout << "adding ampl pt" << std::endl;
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    std::cout << "...and has been initialized" << std::endl;
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    int id = getNextIdForCurvePtTree (amplPtTree);
    addCurvePtToTree (id, freq, ampl, amplPtTree);
    
    updateAmplCurve();
    
    return id;
}

int CabinEqValueTree::addPanPt (const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    int id = getNextIdForCurvePtTree (panPtTree);
    addCurvePtToTree (id, freq, pan, panPtTree);
    
    updatePanCurve();
    
    return id;
}


int CabinEqValueTree::addPhasePt (const float freq, const float phase)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto phasePtTree = valueTree.getChildWithName (idPhaseTree);
    int id = getNextIdForCurvePtTree (phasePtTree);
    addCurvePtToTree (id, freq, phase, phasePtTree);
    
    updatePhaseCurve();
    
    return id;
}
void CabinEqValueTree::removeAmplPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    
    juce::ValueTree nodeToRemove = amplPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        amplPtTree.removeChild (nodeToRemove, nullptr);
    
    updateAmplCurve();
}

void CabinEqValueTree::removePanPt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    
    juce::ValueTree nodeToRemove = panPtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        panPtTree.removeChild (nodeToRemove, nullptr);
    
    updatePanCurve();
}

void CabinEqValueTree::removePhasePt (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto phasePtTree = valueTree.getChildWithName (idPhaseTree);
    
    juce::ValueTree nodeToRemove = phasePtTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        phasePtTree.removeChild (nodeToRemove, nullptr);
    
    updatePhaseCurve();
}

void CabinEqValueTree::updateAmplPt (const int id, const float freq, const float ampl)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto amplPtTree = valueTree.getChildWithName (idAmplTree);
    updateCurvePtInTree (id, freq, ampl, amplPtTree);
    
    updateAmplCurve();
}

void CabinEqValueTree::updatePanPt (const int id, const float freq, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto panPtTree = valueTree.getChildWithName (idPanTree);
    updateCurvePtInTree (id, freq, pan, panPtTree);
    
    updatePanCurve();
}

void CabinEqValueTree::updatePhasePt (const int id, const float freq, const float phase)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto phasePtTree = valueTree.getChildWithName (idPhaseTree);
    updateCurvePtInTree (id, freq, phase, phasePtTree);
    
    updatePhaseCurve();
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
        auto amplPtTree = juce::ValueTree (idAmplTree);
        auto panPtTree = juce::ValueTree (idPanTree);
        auto phasePtTree = juce::ValueTree (idPhaseTree);
        valueTree.addChild (amplPtTree, 0, nullptr);
        valueTree.addChild (panPtTree, 1, nullptr);
        valueTree.addChild (phasePtTree, 2, nullptr);
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
    updateCurves();
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
    std::cout << "UPDATING CURVES" << std::endl;
    updateAmplCurve();
    updatePanCurve();
    updatePhaseCurve();
}

void CabinEqValueTree::updateAmplCurve()
{
    auto pts = getCurvePtsForValueTree (valueTree.getChildWithName (idAmplTree));
    std::cout << "updating ampl curve with " << pts.size() << " points" << std::endl;
    amplCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idAmplTree)));
}

void CabinEqValueTree::updatePanCurve()
{
    panCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idPanTree)));
}

void CabinEqValueTree::updatePhaseCurve()
{
    phaseCurve.updateWithCurvePts (getCurvePtsForValueTree (valueTree.getChildWithName (idPhaseTree)));
}

std::vector<CurvePt> CabinEqValueTree::getCurvePtsForValueTree (juce::ValueTree curvePtValueTree) const
{
    std::vector<CurvePt> curvePts;
    
    if (! curvePtValueTree.isValid())
    {
        std::cout << profileName << ": value tree not valid!" << std::endl;
        return curvePts;
    }
    
    for (const auto& curvePt : curvePtValueTree)
    {
        int id = curvePt.getProperty (idId);
        float freq = curvePt.getProperty (idFreq);
        float val = curvePt.getProperty (idVal);
        curvePts.emplace_back (id, freq, val);
    }
    
    std::cout << profileName << ": getting " << curvePts.size() << " points" << std::endl;
    
    return curvePts;
}
