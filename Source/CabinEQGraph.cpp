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
    updateEQNodes();
    startTimer (5);
}

CabinEQGraph::~CabinEQGraph()
{
    removeListener();
}

void CabinEQGraph::paint (juce::Graphics& g)
{
    g.fillAll (BACKGROUND_COLOR);
    drawCurve (g, curve, 300);
    drawDots (g);
}

void CabinEQGraph::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CabinEQGraph::mouseMove (const juce::MouseEvent &event)
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // If ctrl/alt is held down, start testing
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        listener->testValueAt (freq);
        return;
    }
    
    // Show the ghost node to add if the mouse is on the curve
    if (dbDistanceFromCurve (freq, ampl) <= DIST_TO_ADD_DB)
        addingFreq = freq;
    else
        addingFreq.reset();
    
    // Figure out which node, if any, we're hovering over
    hoveringId = -1;
    std::optional<EQNode> hoveringEQNode = getClosestEQNodeToMouseEvent (event);
    if (hoveringEQNode.has_value())
    {
        hoveringId = hoveringEQNode.value().id;
        targetSelectedDotSize = DOT_SIZE_DRAGGING;
        
        // If we're hovering, we don't want to show the ghost node to add
        if (addingFreq.has_value())
            addingFreq.reset();
    }
    
    // Since ctrl/alt isn't held down, stop testing
    isTestingFreq = false;
    listener->stopTesting();
}

void CabinEQGraph::mouseDown (const juce::MouseEvent &event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Begin to track dragging
    lastDistanceFromDragStartX = 0;
    
    // If we were hovering over a node, we should now drag it
    draggingId = hoveringId;
    if (draggingId != -1)
        targetSelectedDotSize = DOT_SIZE_DRAGGING;
    
    // If we were going to add a node, do so here
    if (addingFreq.has_value() && ! event.mods.isRightButtonDown())
    {
        // Add the curve where we click
        draggingId = listener->addNode (freq, ampl);
        targetSelectedDotSize = DOT_SIZE_DRAGGING;
        addingFreq.reset();
    }
    
    // If we right click and are hovering, delete the node
    if (hoveringId != -1 && event.mods.isRightButtonDown())
    {
        listener->removeNode (hoveringId);
    }
        
    // If we ended up dragging a node, start playing tones
    if (draggingId != -1)
    {
        listener->playValueAt (freq, ampl);
    }
        
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

void CabinEQGraph::addListener (CabinEQGraphListener* listener)
{
    this->listener = listener
}

void CabinEQGraph::removeListener()
{
    this->listener = nullptr;
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
