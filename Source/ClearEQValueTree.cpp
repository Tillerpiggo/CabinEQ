/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ClearEQValueTree.h"

ClearEQValueTree::ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : apvts (apvts), idProfile (identifier), idEQNode ("EQNode"), idId ("id"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan")
{}

const std::vector<EQNode> ClearEQValueTree::getEQNodes() const
{
    if (! hasBeenInitialized) return {};
    
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

const Curve& ClearEQValueTree::getCurve() const
{
    return curve;
}

void ClearEQValueTree::addEQNode (const int id, const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized) return;
    
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (eqNode, nullptr);
    apvts.state = valueTree;
    
    curve.updateWithEQNodes (getEQNodes());
}

int ClearEQValueTree::addEQNode (const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized) return -1;
    
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

void ClearEQValueTree::removeEQNode (const int id)
{
    if (! hasBeenInitialized) return;
    
    juce::ValueTree nodeToRemove = valueTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        valueTree.removeChild (nodeToRemove, nullptr);
    
    curve.updateWithEQNodes (getEQNodes());
}

void ClearEQValueTree::updateEQNode (const int id, const float frequency, const float amplitude, const float pan)
{
    if (! hasBeenInitialized) return;
    
    juce::ValueTree nodeToModify = valueTree.getChildWithProperty (idId, id);
    
    if (nodeToModify.isValid())
    {
        nodeToModify.setProperty (idFrequency, frequency, nullptr);
        nodeToModify.setProperty (idAmplitude, amplitude, nullptr);
        nodeToModify.setProperty (idPan, pan, nullptr);
    }
    
    curve.updateWithEQNodes (getEQNodes());
}

void ClearEQValueTree::resetNodes (const std::vector<EQNode>& eqNodes)
{
    if (! hasBeenInitialized) return;
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    for (const auto& eqNode : eqNodes)
        addEQNode (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan);
    
    curve.updateWithEQNodes (getEQNodes());
}

void ClearEQValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state;

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        apvts.state = valueTree;
    }
    
    hasBeenInitialized = true;
}

void ClearEQValueTree::resetAPVTS (juce::AudioProcessorValueTreeState& apvts)
{
    apvts.state.removeAllChildren (nullptr);
    apvts.state.removeAllProperties (nullptr);
}

void ClearEQValueTree::printValueTree (juce::ValueTree& valueTree) const
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
