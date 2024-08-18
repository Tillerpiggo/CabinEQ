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
    
    int id;
    float frequency;
    float amplitude; // in DB
};
