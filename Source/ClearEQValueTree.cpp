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
    /*
    std::cout << "APVTS ON INIT" << std::endl;
    printValueTree (apvts.state);
    valueTree = apvts.state;//.getChildWithName (idProfile);
//
//    // Initialize value tree if we can't load it
//    if (! valueTree.isValid())
//    {
//        valueTree = juce::ValueTree (idProfile);
//        apvts.state = valueTree;
//        
//        std::cout << "RESETTING TREE" << std::endl;
//        printValueTree (apvts.state);
//        
//        std::cout << "Value Tree Parent: " << valueTree.getType().toString() << std::endl;
//    }
     */
}

const std::vector<EQNode> ClearEQValueTree::getEQNodes() const
{
    std::cout << "GETTING EQ NODES FROM TREE" << std::endl;
    printValueTree (apvts.state);
    
    std::vector<EQNode> eqNodes;
    std::cout << "1" << std::endl;
    if (! valueTree.isValid())
        return eqNodes;
    
    std::cout << "2" << std::endl;
    
    for (const auto& eqNode : valueTree)
    {
        int id = eqNode.getProperty (idId);
        float freq = eqNode.getProperty (idFrequency);
        float ampl = eqNode.getProperty (idAmplitude);
        float pan = eqNode.getProperty (idPan);
        eqNodes.emplace_back (id, freq, ampl, pan);
    }
    
    std::cout << "3" << std::endl;
    
    return eqNodes;
}

void ClearEQValueTree::addNode (const int id, const float frequency, const float amplitude, const float pan)
{
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (eqNode, nullptr);
    apvts.state = valueTree;
}

int ClearEQValueTree::addNode (const float frequency, const float amplitude, const float pan)
{
    std::cout << "Value Tree Parent: " << valueTree.getParent().getType().toString() << std::endl;
    
    std::cout << "APVTS BEFORE ADDING NODE" << std::endl;
    printValueTree (apvts.state);
    
    std::cout << "ValueTree BEFORE ADDING NODE" << std::endl;
    printValueTree (valueTree);
    
    //std::cout << "Adding node" << std::endl;
    
    int id = -1;
    for (const auto& eqNode : valueTree)
    {
        id = std::max ((int) eqNode.getProperty (idId), id);
    }
    
    addNode (id, frequency, amplitude, pan);
    
//    juce::ValueTree eqNode (idEQNode);
//    eqNode.setProperty (idId, id, nullptr);
//    eqNode.setProperty (idFrequency, frequency, nullptr);
//    eqNode.setProperty (idAmplitude, amplitude, nullptr);
//    eqNode.setProperty (idPan, pan, nullptr);
//    valueTree.appendChild (eqNode, nullptr);
    
    //std::cout << "Num nodes: " << valueTree.getNumChildren() << std::endl;
    
//    std::cout << "All nodes" << std::endl;
//    for (const auto& eqNode: getEQNodes())
//        std::cout << "EQNode(id: " << eqNode.id << ", freq: " << eqNode.frequency << ", ampl: " << eqNode.amplitude << ", pan: " << eqNode.pan << ")" << std::endl;
    
//    apvts.state = valueTree;
    
    std::cout << "Added node! Current state:" << std::endl;
    std::cout << "Num nodes: " << apvts.state.getNumChildren() << std::endl;
    printValueTree (apvts.state);
    
    std::cout << "Added node (ValueTree)! Current state:" << std::endl;
    printValueTree (valueTree);
    
    
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

void ClearEQValueTree::initValueTreeFromAPVTS()
{
    std::cout << "Loading APVTS: " << std::endl;
    printValueTree (apvts.state);
    valueTree = apvts.state;
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
