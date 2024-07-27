/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ClearEQValueTree.h"

ClearEQValueTree::ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : idProfile (identifier), idEQNode ("EQNode"), idId ("id"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan"), apvts (apvts)
{
    valueTree = apvts.state.getChildWithName (idProfile);
    
    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        std::cout << "RESETTING TREE" << std::endl;
        valueTree = juce::ValueTree (idProfile);
        //apvts.state.appendChild (valueTree, nullptr);
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
    //std::cout << "Adding node" << std::endl;
    
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (eqNode, nullptr);
    
    //std::cout << "Num nodes: " << valueTree.getNumChildren() << std::endl;
    
//    std::cout << "All nodes" << std::endl;
//    for (const auto& eqNode: getEQNodes())
//        std::cout << "EQNode(id: " << eqNode.id << ", freq: " << eqNode.frequency << ", ampl: " << eqNode.amplitude << ", pan: " << eqNode.pan << ")" << std::endl;
    
    std::cout << "State after adding node" << std::endl;
    std::cout << "Saving state as: " << apvts.state.getType().toString() << std::endl;
    const auto& clearEQTree = apvts.state.getChildWithName (idProfile);
    if (clearEQTree.isValid())
    {
        std::cout << "ClearEQValueTree (numNodes: " << clearEQTree.getNumChildren() << ")" << std::endl;
        juce::Identifier idId ("id");
        juce::Identifier idFrequency ("frequency");
        juce::Identifier idAmplitude ("amplitude");
        if (clearEQTree.getNumChildren() > 0)
        {
            for (const auto& eqNode : clearEQTree)
            {
                float id = eqNode.getProperty (idId);
                float freq = eqNode.getProperty (idFrequency);
                float ampl = eqNode.getProperty (idAmplitude);
                
                std::cout << "EQNode (id: " << id << ", freq: " << freq << ", ampl: " << ampl << ")" << std::endl;
            }
        }
    }
    std::cout << std::endl;
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
