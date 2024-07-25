/*
  ==============================================================================

    CurveValueTree.cpp
    Created: 24 Jul 2024 10:50:20pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "ClearEQValueTree.h"

ClearEQValueTree::ClearEQValueTree (juce::AudioProcessorValueTreeState& apvts, const juce::String& identifier)
    : idProfile (identifier), idSetPoint ("SetPoint"), idFrequency ("frequency"), idAmplitude ("amplitude"), idPan ("pan")
{
    valueTree = apvts.state.getChildWithName (idProfile);
    
    // Initialize value tree if we can't load it
    if (! valueTree.isValid())
    {
        valueTree = juce::ValueTree (idProfile);
        apvts.state.appendChild (valueTree, nullptr);
    }
}

const std::vector<SetPoint> ClearEQValueTree::getSetPoints() const
{
    std::vector<SetPoint> setPoints;
    if (! valueTree.isValid())
        return setPoints;
    
    for (const auto& setPoint : valueTree)
    {
        float freq = setPoint.getProperty (idFrequency);
        float ampl = setPoint.getProperty (idAmplitude);
        float pan = setPoint.getProperty (idPan);
        setPoints.emplace_back (freq, ampl, pan);
    }
    
    return setPoints;
}

void ClearEQValueTree::addNode (const float frequency, const float amplitude, const float pan)
{
    juce::ValueTree setPointNode (idSetPoint);
    setPointNode.setProperty (idFrequency, frequency, nullptr);
    setPointNode.setProperty (idAmplitude, amplitude, nullptr);
    setPointNode.setProperty (idPan, pan, nullptr);
    valueTree.appendChild (setPointNode, nullptr);
}

void ClearEQValueTree::removeNode (const float frequency)
{
    juce::ValueTree nodeToRemove = valueTree.getChildWithProperty (idFrequency, frequency);
    valueTree.removeChild (nodeToRemove, nullptr);
}

void ClearEQValueTree::resetNodes (const std::vector<SetPoint>& setPoints)
{
    valueTree.removeAllChildren (nullptr);
    valueTree.removeAllProperties (nullptr);
    for (const auto& setPoint : setPoints)
        addNode (setPoint.frequency, setPoint.amplitude, setPoint.pan);
}

void ClearEQValueTree::resetAPVTS (juce::AudioProcessorValueTreeState& apvts)
{
    apvts.state.removeAllChildren (nullptr);
    apvts.state.removeAllProperties (nullptr);
}
