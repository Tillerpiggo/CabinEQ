/*
  ==============================================================================

    CabinEQGraph.cpp
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQGraph.h"

CabinEQGraph::CabinEQGraph (Curve& curve) : curve (curve)
{
    
}

CabinEQGraph::~CabinEQGraph()
{
    
}

void CabinEQGraph::paint (juce::Graphics&)
{
    
}

void CabinEQGraph::resized()
{
    
}

void CabinEQGraph::mouseMove (const juce::MouseEvent &event)
{
    
}

void CabinEQGraph::mouseDown (const juce::MouseEvent &event)
{
    
}

void CabinEQGraph::mouseDrag (const juce::MouseEvent &event)
{
    
}

void CabinEQGraph::mouseUp (const juce::MouseEvent &event)
{
    
}

void CabinEQGraph::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    
}

void CabinEQGraph::timerCallback()
{
    
}

float CabinEQGraph::frequencyAtTime (float t) const
{
    
}

float CabinEQGraph::timeAtFrequency (float freq) const
{
    
}

std::pair<float, float> CabinEQGraph::frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const
{
    
}

bool CabinEQGraph::mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const
{

}

float CabinEQGraph::mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const
{
    
}

float CabinEQGraph::dbDistanceFromCurve (const float freq, const float ampl) const
{
    
}

std::optional<EQNode> CabinEQGraph::getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const
{
    
}

void CabinEQGraph::updateEQNodes()
{
    
}
