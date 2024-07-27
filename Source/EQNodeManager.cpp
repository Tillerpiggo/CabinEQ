/*
  ==============================================================================

    EQNodeManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#include "EQNodeManager.h"

EQNodeManager::EQNodeManager (juce::AudioProcessorValueTreeState& apvts)
    : clearEQValueTree (apvts, "ClearEQ")
{
    std::cout << "Initializing EQNodeManager" << std::endl;
    // Get EQ nodes straight from clearEQValueTree
    const auto& eqNodes = clearEQValueTree.getEQNodes();
    curve.updateWithEQNodes (eqNodes);
    std::cout << "Initialized EQNodeManager" << std::endl;
    
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
     
}

const std::vector<EQNode>& EQNodeManager::getNodes() const
{
    return clearEQValueTree.getEQNodes();
}

void EQNodeManager::addEQNode (float frequency, float amplitude, float pan)
{
    clearEQValueTree.addNode (frequency, amplitude, pan);
//    std::cout << "Adding EQNode(id: " << id << ")" << std::endl;
//    eqNodes.emplace_back (id, amplitude, frequency, pan);
    numNodes++;
}

void EQNodeManager::removeEQNode (int id)
{
    std::cout << "Removing eq node: " << id << std::endl;
//    for (int i = 0; i < eqNodes.size(); ++i)
//        if (eqNodes[i].id == id)
//            eqNodes.erase (eqNodes.begin() + i);
    clearEQValueTree.removeNode (id);
    numNodes--;
}

void EQNodeManager::updateEQNode (int id, float frequency, float amplitude, float pan)
{
//    for (int i = 0; i < eqNodes.size(); ++i)
//    {
//        if (eqNodes[i].id == id)
//        {
//            eqNodes[i].frequency = frequency;
//            eqNodes[i].amplitude = amplitude;
//            eqNodes[i].pan = pan;
//        }
//    }
    clearEQValueTree.updateNode (id, frequency, amplitude, pan);
    const auto& eqNodes = clearEQValueTree.getEQNodes();
    curve.updateWithEQNodes (eqNodes);
}

void EQNodeManager::loadFromAPVTS()
{
    std::cout << "INIT VALUE TREE FROM APVTS" << std::endl;
    clearEQValueTree.initValueTreeFromAPVTS();
    
//    eqNodes.clear();
//    for (const auto& eqNode : clearEQValueTree.getEQNodes())
//    {
//        std::cout << "EQNode(id: " << eqNode.id << ", freq: " << eqNode.frequency << ")" << std::endl;
//    }
    
    //eqNodes = clearEQValueTree.getEQNodes();
    std::cout << "pushing back" << std::endl;
//    eqNodes.push_back (EQNode (10, 1000, 0, 0));
    std::cout << "pushed back" << std::endl;
}

const Curve& EQNodeManager::getCurve() const
{
    return curve;
}
*/
