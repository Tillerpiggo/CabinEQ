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
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // If we're dragging a node, update it to our mouse position
    listener->updateNode (draggingId, freq, ampl);
    listener->playValueAt (freq, ampl);
    
    // If we're not dragging a node, we're dragging in the blackspace and should drag the curve itself
    if (draggingId == -1)
    {
        float minFreqShowing_t = timeAtFrequency (minFreqShowing);
        float maxFreqShowing_t = timeAtFrequency (maxFreqShowing);
        float dt = (static_cast<float> (event.getDistanceFromDragStartX()) / - lastDistanceFromDragStartX) / getWidth();
        
        float projectedMinFreqShowing = frequencyAtTime (minFreqShowing_t - dt);
        float projectedMaxFreqShowing = frequencyAtTime (maxFreqShowing_t - dt);
        
        // Apply changes if we are within the bounds
        if (projectedMinFreqShowing >= MIN_FREQ &&
            projectedMaxFreqShowing <= MAX_FREQ)
        {
            minFreqShowing = projectedMinFreqShowing;
            maxFreqShowing = projectedMaxFreqShowing;
        }
    }
}

void CabinEQGraph::mouseUp (const juce::MouseEvent &event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Update dragging node a final time
    listener->updateNode (draggingId, freq, ampl);
    
    // Change the dot size
    targetSelectedDotSize = DOT_SIZE_DEFAULT;
    
    // We stopped dragging, so stop playing tones
    listener->stopPlaying();
}

void CabinEQGraph::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    // Useful constants
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Math to figure out how much left/right side of window should move
    float p = 1 - (wheel.deltaY); // % change in window width
    float dx = wheel.deltaX * -0.3f; // amount window is shifted horizontally
    float t = timeAtFrequency (freq); // the position of your mouse, in linear space
    float leftChunkSize = t;
    float rightChunkSize = 1 - t;
    float leftSideOfWindow = t - (leftChunkSize * p) + dx;
    float rightSideOfWindow = t + (rightChunkSize * p) + dx;
    
    // Where we're projected to scroll to
    float projectedMinFreqVal = frequencyAtTime (leftSideOfWindow);
    float projectedMaxFreqVal = frequencyAtTime (rightSideOfWindow);
    
    // Limit scrolling to within MIN_FREQ and MAX_FREQ
    minFreqShowing = projectedMinFreqVal >= MIN_FREQ ? frequencyAtTime (leftSideOfWindow) : MIN_FREQ;
    maxFreqShowing = projectedMaxFreqVal <= MAX_FREQ ? frequencyAtTime (rightSideOfWindow) : MAX_FREQ;
}

void CabinEQGraph::timerCallback()
{
    repaint();
}

void CabinEQGraph::addListener (CabinEQGraphListener* listener)
{
    this->listener = listener;
}

void CabinEQGraph::removeListener()
{
    this->listener = nullptr;
}

// =============================================
float CabinEQGraph::frequencyAtTime (float t) const
{
    // Scale logarithmically based on the visible window
    float logMinFreqShowing = std::log (minFreqShowing);
    float logMaxFreqShowing = std::log (maxFreqShowing);
    float freq = std::exp (logMinFreqShowing + t * (logMaxFreqShowing - logMinFreqShowing));
    return freq;
}

float CabinEQGraph::timeAtFrequency (float freq) const
{
    // Scale back to linear based on the visible window
    float logMinFreqShowing = std::log (minFreqShowing);
    float logMaxFreqShowing = std::log (maxFreqShowing);
    float logFreq = std::log (freq);
    float t = (logFreq - logMinFreqShowing) / (logMaxFreqShowing - logMinFreqShowing);
    return t;
}

std::pair<float, float> CabinEQGraph::frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const
{
    // Get mouse coords
    float x = event.getPosition().x;
    float y = event.getPosition().y;
    
    // Calculate frequency of mouse event
    float freq = frequencyAtTime (x / getWidth());
    
    // Calculate amplitude of mouse event
    float normalizedY = y / getHeight();
    float ampl = (1.0f - normalizedY) * 48.0f - 24.0f;
    
    // Compensate for tilt
    float compensationDB = -4.5 * std::log2 (freq / 1000.0f);
    ampl += compensationDB;
    
    // Bound freq/ampl inside the visible window
    freq = std::max (std::min (freq, maxFreqShowing), minFreqShowing);
    ampl = std::min (std::max (ampl, -24.0f + compensationDB), 24.0f + compensationDB);
    
    return { freq, ampl };
}

float CabinEQGraph::mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const
{
    // Calculate distance based on arbitrary scale factors that weigh freq and ampl about the same
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    float dx = std::abs (timeAtFrequency (freq) - timeAtFrequency (eqNode.frequency));
    float dy = std::abs (ampl - eqNode.amplitude);
    dx *= 39;
    dy *= 0.5;
    
    float distance = std::sqrt (dx * dx + dy * dy);
    return distance;
}

float CabinEQGraph::dbDistanceFromCurve (const float freq, const float ampl) const
{
    float curveGainAtFreq = curve.valueAtFrequency (freq).first.real();
    float curveDBAtFreq = juce::Decibels::gainToDecibels (curveGainAtFreq);
    return std::abs (ampl - curveDBAtFreq);
}

std::optional<EQNode> CabinEQGraph::getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const
{
    float minDist = 10.0f; // arbitrary # higher than HOVER_MIN_DIST
    std::optional<EQNode> closestEQNode;
    for (const auto& eqNode : eqNodes)
    {
        float dist = mouseEventEQNodeDistance (event, eqNode);
        if (dist < std::min (minDist, HOVER_MIN_DIST))
        {
            minDist = dist;
            closestEQNode = eqNode;
        }
    }
    
    return closestEQNode;
}

void CabinEQGraph::updateEQNodes()
{
    eqNodes = listener->getEQNodes();
}
