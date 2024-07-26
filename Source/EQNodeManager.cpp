/*
  ==============================================================================

    EQNodeManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "EQNodeManager.h"

EQNodeManager::EQNodeManager (juce::AudioProcessorValueTreeState& apvts)
    : clearEQValueTree (apvts, "ClearEQ")
{
    // Get EQ nodes straight from clearEQValueTree
    this->eqNodes = clearEQValueTree.getEQNodes();
    curve.updateWithEQNodes (eqNodes);
    
    /*
    int id = 0;
    
    // Generate NUM_PTS evenly spaced frequencies from 20 to 15000khz
    eqNodes.clear();
    eqNodes.reserve (NUM_PTS);
    
    float startFreq = 50.0f;
    float endFreq = 950.0f;

    float logStart = std::log10(startFreq);
    float logEnd = std::log10(endFreq);
    float step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        float logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        eqNodes.emplace_back (id, freq, 0, 0);
        id++;
    }
    
    startFreq = 1050.0f;
    endFreq = 7500.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        eqNodes.emplace_back (id, freq, 0, 0);
        id++;
    }
    
    startFreq = 8000.0f;
    endFreq = 18000.0f;

    logStart = std::log10(startFreq);
    logEnd = std::log10(endFreq);
    step = (logEnd - logStart) / (NUM_PTS / 3 - 1);

    for (int i = 0; i < NUM_PTS / 3; ++i) {
        double logFreq = logStart + i * step;
        float freq = std::pow (10, logFreq);
        eqNodes.emplace_back (id, freq, 0, 0);
        id++;
    }
     */
}

void EQNodeManager::addEQNode (float frequency, float amplitude, float pan)
{
    // The next id is one higher than the highest id in the list
    int id = -1;
    for (EQNode eqNode : eqNodes)
        id = std::max (eqNode.id, id);
    id++;
    eqNodes.emplace_back (id, amplitude, frequency, pan);
    clearEQValueTree.addNode (id, frequency, amplitude, pan);
}

void EQNodeManager::removeEQNode (int id)
{
    for (int i = 0; i < eqNodes.size(); ++i)
        if (eqNodes[i].id == id)
            eqNodes.erase (eqNodes.begin() + i);
    clearEQValueTree.removeNode (id);
}

void EQNodeManager::updateEQNode (int id, float frequency, float amplitude, float pan)
{
    for (int i = 0; i < eqNodes.size(); ++i)
    {
        if (eqNodes[i].id == id)
        {
            eqNodes[i].frequency = frequency;
            eqNodes[i].amplitude = amplitude;
            eqNodes[i].pan = pan;
        }
    }
    
    curve.updateWithEQNodes (eqNodes);
    clearEQValueTree.updateNode (id, frequency, amplitude, pan);
}

const Curve& EQNodeManager::getCurve() const
{
    return curve;
}
