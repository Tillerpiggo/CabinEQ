/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQValueTree.h"

CabinEQValueTree::CabinEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : apvts (apvts), idProfile (identifier), idLeftCurve ("LeftCurve"), idRightCurve ("RightCurve"), idEQNode ("EQNode"), idId ("id"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan")
{}

const std::vector<EQNode> CabinEQValueTree::getEQNodes (Channel channel) const
{
    auto curveValueTree = valueTreeForChannel (channel);
    
    std::vector<EQNode> eqNodes;
    if (! curveValueTree.isValid())
        return eqNodes;
    
    for (const auto& eqNode : curveValueTree)
    {
        int id = eqNode.getProperty (idId);
        float freq = eqNode.getProperty (idFrequency);
        float ampl = eqNode.getProperty (idAmplitude);
        float pan = eqNode.getProperty (idPan);
        eqNodes.emplace_back (id, freq, ampl, pan);
    }
    
    return eqNodes;
}

Curve& CabinEQValueTree::getCurve (Channel channel)
{
    switch (channel)
    {
        case Channel::LEFT:
            return leftCurve;
        case Channel::RIGHT:
            return rightCurve;
    }
}

void CabinEQValueTree::addEQNode (const int id, const float frequency, const float amplitude, const float pan, Channel channel)
{
    if (! hasBeenInitialized) return;
    
    juce::ValueTree eqNode (idEQNode);
    eqNode.setProperty (idId, id, nullptr);
    eqNode.setProperty (idFrequency, frequency, nullptr);
    eqNode.setProperty (idAmplitude, amplitude, nullptr);
    eqNode.setProperty (idPan, pan, nullptr);
    
    valueTreeForChannel (channel).appendChild (eqNode, nullptr);
    
    printValueTree (valueTree);
    printValueTree (apvts.state.getChildWithName (idProfile));
    
    getCurve (channel).updateWithEQNodes (getEQNodes (channel));
}

int CabinEQValueTree::addEQNode (const float frequency, const float amplitude, const float pan, Channel channel)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto curveValueTree = valueTreeForChannel (channel);
    
    int id = -1;
    for (const auto& eqNode : curveValueTree)
    {
        id = std::max ((int) eqNode.getProperty (idId), id);
    }
    id++;
    
    addEQNode (id, frequency, amplitude, pan, channel);
    
    getCurve (channel).updateWithEQNodes (getEQNodes (channel));
    
    return id;
}

void CabinEQValueTree::removeEQNode (const int id, Channel channel)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    auto curveValueTree = valueTreeForChannel (channel);
    juce::ValueTree nodeToRemove = curveValueTree.getChildWithProperty (idId, id);
    if (nodeToRemove.isValid())
        curveValueTree.removeChild (nodeToRemove, nullptr);
    
    getCurve (channel).updateWithEQNodes (getEQNodes (channel));
}

void CabinEQValueTree::updateEQNode (const int id, const float frequency, const float amplitude, const float pan, Channel channel)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    juce::ValueTree nodeToModify = valueTreeForChannel (channel).getChildWithProperty (idId, id);
    
    if (nodeToModify.isValid())
    {
        nodeToModify.setProperty (idFrequency, frequency, nullptr);
        nodeToModify.setProperty (idAmplitude, amplitude, nullptr);
        nodeToModify.setProperty (idPan, pan, nullptr);
    }
    
    leftCurve.updateWithEQNodes (getEQNodes (Channel::LEFT));
    rightCurve.updateWithEQNodes (getEQNodes (Channel::RIGHT));
}

void CabinEQValueTree::resetNodes (const std::vector<EQNode>& eqNodes)
{
    if (! hasBeenInitialized)
        initValueTreeFromAPVTS();
    
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    for (const auto& eqNode : eqNodes)
    {
        addEQNode (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan, Channel::LEFT);
        addEQNode (eqNode.id, eqNode.frequency, eqNode.amplitude, eqNode.pan, Channel::RIGHT);
    }
    
    leftCurve.updateWithEQNodes (getEQNodes (Channel::LEFT));
    rightCurve.updateWithEQNodes (getEQNodes (Channel::RIGHT));
}

void CabinEQValueTree::initValueTreeFromAPVTS()
{
    valueTree = apvts.state.getChildWithName (idProfile);

    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        auto leftValueTree = juce::ValueTree (idLeftCurve);
        auto rightValueTree = juce::ValueTree (idRightCurve);
        valueTree.addChild (leftValueTree, 0, nullptr);
        valueTree.addChild (rightValueTree, 1, nullptr);
        apvts.state.addChild (valueTree, -1, nullptr);
    }
    
    leftCurve.updateWithEQNodes (getEQNodes (Channel::LEFT));
    rightCurve.updateWithEQNodes (getEQNodes (Channel::RIGHT));
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

juce::ValueTree CabinEQValueTree::valueTreeForChannel (Channel channel) const
{
    switch (channel)
    {
        case Channel::LEFT:
            return valueTree.getChildWithName (idLeftCurve);
        case Channel::RIGHT:
            return valueTree.getChildWithName (idRightCurve);
    }
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
