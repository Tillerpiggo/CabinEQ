/*
  ==============================================================================

    EQNode.h
    Created: 24 Jul 2024 10:37:34pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

/// This stores the raw data for each set point in a given EQ curve.
struct EQNode
{
    EQNode (int id, float frequency, float amplitude)
        : id (id), frequency (frequency), amplitude (amplitude)
    {}
    
    // Default constructor for convenience
    EQNode()
        : id (-1), frequency (0), amplitude (0)
    {}
    
    int id;
    float frequency;
    float amplitude; // in DB
};
