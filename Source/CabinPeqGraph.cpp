/*
  ==============================================================================

    CabinPeqGraph.cpp
    Created: 10 Oct 2024 4:30:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinPeqGraph.h"

CabinPeqGraph::CabinPeqGraph()
{
}

CabinPeqGraph::~CabinPeqGraph()
{
    removeListener();
}

void CabinPeqGraph::setBandProfile (BandProfile& bandProfile)
{
    this->bandProfile = bandProfile;
}

void CabinPeqGraph::paint (juce::Graphics& g)
{
    g.setColour (BACKGROUND_COLOR);
    g.fillRect (getBoundsInParent());
    
    if (bandProfile.has_value())
    {
        drawCurve (g, bandProfile->get());
        drawDots (g, bandProfile->get());
    }
}

void CabinPeqGraph::resized()
{
    setBounds (getBoundsInParent());
}

void CabinPeqGraph::mouseMove (const juce::MouseEvent &event)
{
    soloNoisePatternIfAppropriate (event);
    updateHoveringStatus (event);
}

void CabinPeqGraph::mouseDown (const juce::MouseEvent &event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    soloNoisePatternIfAppropriate (event);
    
    // Begin to track dragging
    lastDistanceFromDragStartX = 0;
    
    // If we were hovering over a band node, we should now drag it
    if (draggingId != -1)
        selectedDotSize = DOT_SIZE_DRAGGING;
    
    // If we were going to add a band, do so here
    if (isHoveringOnCenterLine && ! event.mods.isRightButtonDown())
    {
        // Add the band where we click
        draggingId = addBand (freq, ampl, DEFAULT_BANDWIDTH);
        selectedDotSize = DOT_SIZE_DRAGGING;
        isHoveringOnCenterLine = false;
    }
    
    // If we right click and were hovering, delete the band
    if (hoveringId != -1 && event.mods.isRightButtonDown())
        removeBand (hoveringId);
    
    // If we are dragging a band, start playing an appropriate noise pattern
    if (draggingId != -1)
    {
        updateBand (draggingId, freq, ampl, DEFAULT_BANDWIDTH);
        startNoisePatternAt (draggingId);
    }
}

void CabinEqGraph::mouseDrag (const juce::MouseEvent& event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    soloNoisePatternIfAppropriate (event);
    
    // If we're dragging a node, update it to our mouse position
    if (draggingId != -1)
    {
        updateBand (draggingId, freq, ampl, DEFAULT_BANDWIDTH);
        updateNoisePatternAt (draggingId);
    }
    
    // If we're not dragging a node, we're dragging in the blackspace and should drag the graph itself
    else
    {
        float minFreqShowingTime = timeAtFrequency (minFreqShowig);
        float maxFreqShowingTime = timeAtFrequency (maxFreqShowing);
        float timeChange = (static_cast<float> (event.getDistanceFromDragStart()) / -lastDistanceFromDragStartX) / getWidth();
        
        float projectedMinFreqShowing = frequencyAtTime (minFreqShowingTime - timeChange);
        float projectedMaxFreqShowing = frequencyAtTime (maxFreqShowingTime - timeChange);
        
        // Apply changes if we are within the bounds of the graph
        if (projectedMinFreqShowing >= MIN_FREQ && projectedMaxFreqShowing <= MAX_FREQ)
        {
            minFreqShowing = projectedMinFreqShowing;
            maxFreqShowing = projectedMaxFreqShowing;
        }
    }
}

void CabinEqGraph::mouseUp (const juce::MouseEvent& event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Update dragging node a final time
    updateBand (draggingId, freq, ampl, DEFAULT_BANDWIDTH);
    draggingId = -1;
    
    // Change the dot size back to normal
    selectedDotSize = DOT_SIZE_DEFAULT;
    
    // We stopped dragging, so stop playing the noise pattern
    stopNoisePattern();
}

void CabinPeqGraph::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Useful constants
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
    soloNoisePatternIfAppropriate (event);
    
    // Math to figure out how much the left/right side of the window should move
    float windowWidthChangePercent = 1 - (wheel.deltaY);
    float windowHorizontalChange = wheel.deltaX * -0.3f;
    float mouseHorizontalTime = timeAtFrequency (freq);
    float leftChunkSize = mouseHorizontalTime;
    float rightChunkSize = 1 - mouseHorizontalTime;
    float leftSideOfWindow = mouseHorizontalTime - (leftChunkSize * windowWidthChangePercent) + windowHorizontalChange;
    float rightSideOfWindow = mouseHorizontalTime + (rightChunkSize * windowWidthChangePercent) + windowHorizontalChange;
    
    // The bounds we're projected to scroll to
    float projectedMinFreq = frequencyAtTime (leftSideOfWindow);
    float projectedMaxFreq = frequencyAtTime (rightSideOfWindow);
    
    // Limit scrolling to within MIN_FREQ and MAX_FREQ
    minFreqShowing = std::max (projectedMinFreq, MIN_FREQ);
    maxFreqShowing = std::min (projectedMaxFreq, MAX_FREQ);
    
    updateHoveringStatus (event);
}

bool CabinPeqGraph::keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent)
{
    setNoisePatternSolo (juce::KeyPress::isKeyCurrentlyDown (juce::ModifierKeys::ctrlModifier) || juce::KeyPress::isKeyCurrentlyDown (juce::ModifierKeys::altModifier));
    return true;
}

bool CabinPeqGraph::keyStateChanged (bool isKeyDown, juce::Component *originatingComponent)
{
    setNoisePatternSolo (juce::KeyPress::isKeyCurrentlyDown (juce::ModifierKeys::ctrlModifier) || juce::KeyPress::isKeyCurrentlyDown (juce::ModifierKeys::altModifier));
    return true;
}

void CabinPeqGraph::timerCallback()
{
    repaint();
}

void CabinPeqGraph::addListener (Listener* listener)
{
    this->listener = listener;
}

void CabinPeqGraph::removeListener()
{
    this->listener = nullptr;
}

void CabinEqGraph::setGrayscale (bool grayscale)
{
    this->grayscale = grayscale;
}

// =============================================
