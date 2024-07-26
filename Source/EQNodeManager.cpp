/*
  ==============================================================================

    EQNodeManager.cpp
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "EQNodeManager.h"

EQNodeManager::EQNodeManager()
{
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
    
    curve.updateWithEQNodes (eqNodes);
}

void EQNodeManager::setNodeAt (int idx, EQNode node)
{
    if (idx < 0 || idx >= getNumNodes())
        return;
    eqNodes[idx] = node;
    curve.updateWithEQNodes (eqNodes);
}

void EQNodeManager::setAmplitudeAt (int idx, float newAmplitude)
{
    if (idx < 0 || idx >= getNumNodes())
        return;
    eqNodes[idx].amplitude = newAmplitude;
    curve.updateWithEQNodes (eqNodes);
}

void EQNodeManager::setPanAt (int idx, float newPan)
{
    if (idx < 0 || idx >= getNumNodes())
        return;
    eqNodes[idx].pan = newPan;
    curve.updateWithEQNodes (eqNodes);
}

const Curve& EQNodeManager::getCurve() const
{
    return curve;
}
