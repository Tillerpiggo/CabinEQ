/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ClearEQValueTree.h"

ClearEQValueTree::ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : idProfile (identifier), idEQNode ("EQNode"), idId ("id"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan")
{
    valueTree = apvts.state.getChildWithName (idProfile);
    
    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        apvts.state.appendChild (valueTree, nullptr);
    }
}

const std::vector<EQNode> ClearEQValueTree::getEQNodes() const
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

void ClearEQValueTree::addNode (const int id, const float frequency, const float amplitude, const float pan)
{
    std::cout << "Adding node" << std::endl;
    
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (eqNode, nullptr);
    
    std::cout << "All nodes" << std::endl;
    for (const auto& eqNode: getEQNodes())
        std::cout << "EQNode(id: " << eqNode.id << ", freq: " << eqNode.frequency << ", ampl: " << eqNode.amplitude << ", pan: " << eqNode.pan << ")" << std::endl;
}

void ClearEQValueTree::removeNode (const int id)
{
    juce::ValueTree nodeToRemove = valueTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        valueTree.removeChild (nodeToRemove, nullptr);
}

void ClearEQValueTree::updateNode (const int id, const float frequency, const float amplitude, const float pan)
{
    juce::ValueTree nodeToModify = valueTree.getChildWithProperty (idId, id);
    
    if (nodeToModify.isValid())
    {
        nodeToModify.setProperty (idFrequency, frequency, nullptr);
        nodeToModify.setProperty (idAmplitude, amplitude, nullptr);
        nodeToModify.setProperty (idPan, pan, nullptr);
    }
}

void ClearEQValueTree::resetNodes (const std::vector<EQNode>& eqNodes)
{
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    for (const auto& eqNode : eqNodes)
        addNode (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan);
}

void ClearEQValueTree::resetAPVTS (juce::AudioProcessorValueTreeState& apvts)
{
    apvts.state.removeAllChildren (nullptr);
    apvts.state.removeAllProperties (nullptr);
}
