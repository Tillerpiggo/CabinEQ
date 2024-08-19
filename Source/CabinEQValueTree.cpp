/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQValueTree.h"

CabinEQValueTree::CabinEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : apvts (apvts), idProfile (identifier), idEQNode ("EQNode"), idId ("id"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan")
{}

const std::vector<EQNode> CabinEQValueTree::getEQNodes() const
{
    std::vector<EQNode> eqNodes;
    if (! valueTree.isValid())
        return eqNodes;
    
    for (const auto& eqNode : valueTree)
    {
        int id = eqNode.getProperty (idId);
        float freq = eqNode.getProperty (idFrequency);
        float ampl = eqNode.getProperty (idAmplitude);
        float pan = eqNode.getProperty (idPan);
        eqNodes.emplace_back (id, freq, ampl, pan);
    }
    
    return eqNodes;
}

Curve& CabinEQValueTree::getCurve()
{
    return curve;
}

void CabinEQValueTree::addEQNode (const int id, const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized) return;
    
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (eqNode, nullptr);
    
    printValueTree (valueTree);
    printValueTree (apvts.state.getChildWithName (idProfile));
//    apvts.state.getChildWithName (idProfile) = valueTree;
//    apvts.state = valueTree;
    
    curve.updateWithEQNodes (getEQNodes());
}

int CabinEQValueTree::addEQNode (const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    int id = -1;
    for (const auto& eqNode : valueTree)
    {
        id = std::max ((int) eqNode.getProperty (idId), id);
    }
    id++;
    
    addEQNode (id, frequency, amplitude, pan);
    
    curve.updateWithEQNodes (getEQNodes());
    
    return id;
}

void CabinEQValueTree::removeEQNode (const int id)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    juce::ValueTree nodeToRemove = valueTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        valueTree.removeChild (nodeToRemove, nullptr);
    
    curve.updateWithEQNodes (getEQNodes());
}

void CabinEQValueTree::updateEQNode (const int id, const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    juce::ValueTree nodeToModify = valueTree.getChildWithProperty (idId, id);
    
    if (nodeToModify.isValid())
    {
        nodeToModify.setProperty (idFrequency, frequency, nullptr);
        nodeToModify.setProperty (idAmplitude, amplitude, nullptr);
        nodeToModify.setProperty (idPan, pan, nullptr);
    }
    
    curve.updateWithEQNodes (getEQNodes());
}

void CabinEQValueTree::resetNodes (const std::vector<EQNode>& eqNodes)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    for (const auto& eqNode : eqNodes)
        addEQNode (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan);
    
    curve.updateWithEQNodes (getEQNodes());
}

void CabinEQValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithName (idProfile);

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    
    curve.updateWithEQNodes (getEQNodes());
    hasBeenInitialized = true;
}

const juce::String CabinEQValueTree::getName() const
{
    return idProfile.toString();
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
