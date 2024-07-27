/*
  ==============================================================================

    EQNodeManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/


#pragma once

#include <JuceHeader.h>
#include "ClearEQValueTree.h"
#include "EQNode.h"
#include "Curve.h"

/// This class manages reading and writing data in ClearEQ. Adding/removing profiles, changing set points, and a useful interface for getting relevant data is handled here.
class EQNodeManager
{
public:
    EQNodeManager (juce::AudioProcessorValueTreeState& apvts);
    
    const std::vector<EQNode>& getNodes() const;
    const int getNumNodes() const { return numNodes; }
    
    void addEQNode (float frequency, float amplitude, float pan);
    void removeEQNode (int id);
    void updateEQNode (int id, float frequency, float amplitude, float pan);
    
    void loadFromAPVTS(); // loads nodes from whatever the apvts has stored right now
    
    const Curve& getCurve() const;
    
    static const int NUM_PTS = 50;
    
private:
    ClearEQValueTree clearEQValueTree;
    int numNodes = 0;
    
//    std::vector<EQNode> eqNodes;
    Curve curve;
};
