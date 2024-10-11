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

void CabinPeqGraph::mouseDrag (const juce::MouseEvent& event)
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
        float minFreqShowingTime = timeAtFrequency (minFreqShowing);
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

void CabinPeqGraph::mouseUp (const juce::MouseEvent& event)
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

void CabinPeqGraph::setGrayscale (bool isGrayscale)
{
    this->isGrayscale = isGrayscale;
}

// =============================================
void CabinPeqGraph::drawCurve (juce::Graphics& g, BandProfile& bandProfile)
{
    // TODO - we won't draw the curve for now
}

void CabinPeqGraph::drawDots (juce::Graphics& g, BandProfile& bandProfile)
{
    // Pseudocode
    
    // Get all bands in band profile
    
    // for each band, draw a corresponding point
    
    // add necessary exceptions for the dragging/hovering band id, the ghost node, etc.
    
    // reference CabinEqGraph::drawDots for more specific outline
}

void CabinPeqGraph::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour)
{
    // Draw dot
    g.fillEllipse (point.x - dotRadius - DOT_PADDING, point.y - dotRadius - DOT_PADDING, (dotRadius + DOT_PADDING) * 2, (dotRadius + DOT_PADDING) * 2);
    
    // Draw the center of the dot
    g.setColour (dotColour);
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

void CabinPeqGraph::updateHoveringStatus (const juce::MouseEvent& event)
{
    // If there is no band profile, we can't determine hovering status (and the profile hasn't loaded yet)
    // TODO: we might want to make an exception for hovering over the center line, but this optimization is minor for now
    if (! bandProfile.has_value())
        return;
    
    // Helpful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Show the ghost node to add if the mouse is on the center line
    if (ampl <= DIST_TO_ADD_DB)
        isHoveringOnCenterLine = true;
    else
        isHoveringOnCenterLine = false;
    
    // Figure out which node, if any, we're hovering over
    hoveringId = -1;
    auto hoveringBand = getClosestBandToMouseEvent (event);
    if (hoveringBand.has_value())
    {
        hoveringId = hoveringBand.value().id;
        selectedDotSize = DOT_SIZE_DRAGGING;
        
        // If we'er hovering, we don't want to show the ghost node to add
        if (isHoveringOnCenterLine)
            isHoveringOnCenterLine = false;
    }
}

juce::Colour CabinPeqGraph::getColourForFrequency (float frequency)
{
    if (isGrayscale)
        return juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
    
    return juce::Colour::fromFloatRGBA (0.5f, 0.5f, 0.5f, 1.0f);
}

juce::Point<float> CabinPeqGraph::coordsForFrequencyAndAmplitude (float freq, float ampl)
{
    // Calculate (x, y) coords and return
    float x = getWidth() * timeAtFrequency (freq);
    float y = getHeight() * (1.0f - (ampl - MIN_DB) / (MAX_DB - MIN_DB));
    
    return { x + getX(), y + getY() };
}

float CabinPeqGraph::frequencyAtTime (float t) const
{
    // Scale logarithmically based on the visible window
    float logMinFreqShowing = std::log (minFreqShowing);
    float logMaxFreqShowing = std::log (maxFreqShowing);
    float freq = std::exp (logMinFreqShowing + t * (logMaxFreqShowing - logMinFreqShowing));
    return freq;
}

float CabinPeqGraph::timeAtFrequency (float freq) const
{
    // Scale back to linear based on the visible window
    float logMinFreqShowing = std::log (minFreqShowing);
    float logMaxFreqShowing = std::log (maxFreqShowing);
    float logFreq = std::log (freq);
    float t = (logFreq - logMinFreqShowing) / (logMaxFreqShowing - logMinFreqShowing);
    return t;
}

std::pair<float, float> CabinPeqGraph::frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const
{
    // Get mouse coords
    float x = event.getPosition().x - getX();
    float y = event.getPosition().y - getY();
    
    // Calculate frequency of mouse event
    float freq = frequencyAtTime (x / getWidth());
    
    // Calculate amplitude of mouse event
    float normalizedY = y / getHeight();
    float ampl = (1.0f - normalizedY) * (MAX_DB - MIN_DB) + MIN_DB;
    
    // Bound freq/ampl inside the visible window
    freq = std::max (std::min (freq, maxFreqShowing), minFreqShowing);
    ampl = std::min (std::max (ampl, MIN_DB), MAX_DB);
    
    return { freq, ampl };
}

float CabinPeqGraph::mouseEventDistanceFromBand (const juce::MouseEvent& event, Band band) const
{
    // Calculate distance based on arbitrary scale factors that weight freq and ampl about the same
    // TODO: could theoretically improve the precision of this
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    float dx = std::abs (timeAtFrequency (freq) - timeAtFrequency (band.freq));
    float dy = std::abs (ampl - band.ampl);
    dx *= 39;
    dy *= 0.5;
    
    float distance = std::sqrt (dx * dx + dy * dy);
    return distance;
}


