/*
  ==============================================================================

    EQNodeManager.h
    Created: 10 Jul 2024 3:39:46pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ClearEQvalueTree.h"
#include "EQNode.h"
#include "Curve.h"

/// This class manages reading and writing data in ClearEQ. Adding/removing profiles, changing set points, and a useful interface for getting relevant data is handled here.
class EQNodeManager
{
public:
    EQNodeManager (juce::AudioProcessorValueTreeState& apvts);
    
    const std::vector<EQNode>& getNodes() const { return eqNodes; }
    const int getNumNodes() const { return static_cast<int> (eqNodes.size()); }
    
    void addEQNode (float frequency, float amplitude, float pan);
    void removeEQNode (int id);
    void updateEQNode (int id, float frequency, float amplitude, float pan);
    
    const Curve& getCurve() const;
    
    int indexForFrequency (float frequency) const
    {
        for (int i = 0; i < eqNodes.size(); ++i)
        {
            if (frequency == eqNodes.at (i).frequency)
            {
                return i;
            }
        }
        
        return -1;
    }
    
    static const int NUM_PTS = 60;
    
private:
    ClearEQValueTree clearEQValueTree;
    
    std::vector<EQNode> eqNodes;
    Curve curve;
};
