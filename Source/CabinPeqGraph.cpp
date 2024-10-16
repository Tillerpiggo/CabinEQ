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
    startTimer (5);
}

CabinPeqGraph::~CabinPeqGraph()
{
    removeListener();
}

void CabinPeqGraph::setBands (std::vector<Band> bands)
{
    this->bands = bands;
    this->curve.updateWithBands (bands);
}

void CabinPeqGraph::paint (juce::Graphics& g)
{
    g.setColour (BACKGROUND_COLOR);
    g.fillRect (getBoundsInParent());
    
    drawLines (g); // draw lines before so that they are drawn over
    drawCurve (g);
    drawDots (g);
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
    auto coords = getEventCoords (event);
    
    soloNoisePatternIfAppropriate (event);
    
    // Begin to track dragging
    lastDistanceFromDragStartX = 0;
    
    // If we were hovering over a band node, we should now drag it
    draggingId = hoveringId;
    if (draggingId != -1)
    {
        selectedDotSize = DOT_SIZE_DRAGGING;
        startDragPosition = coords;
        lastDragPosition = coords;
        dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
        dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
    }
    
    // If we were going to add a band, do so here
    else if (addingFreq.has_value() && ! event.mods.isRightButtonDown())
    {
        // Add the band where we click
        draggingId = addBand (freq, ampl, DEFAULT_BANDWIDTH);
        selectedDotSize = DOT_SIZE_DRAGGING;
        startDragPosition = coords;
        lastDragPosition = coords;
        startDragBandwidth = DEFAULT_BANDWIDTH;
        dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
        dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
        addingFreq.reset();
    }
    
    // If we right click and were hovering, delete the band
    if (hoveringId != -1 && event.mods.isRightButtonDown())
        removeBand (hoveringId);
    
    // If we are dragging a band, start playing an appropriate noise pattern
    if (draggingId != -1)
    {
        updateBand (draggingId, freq, ampl, startDragBandwidth);
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
        updateBandFromDrag (event);
        updateNoisePatternAt (draggingId);
        
        lastDragPosition = getEventCoords (event);
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
    
    updateBandFromDrag (event);
    draggingId = -1;
    
    dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
    dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
    
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
void CabinPeqGraph::drawCurve (juce::Graphics& g)
{
    // Get the gradient for the curve
    juce::ColourGradient curveGradient = getCurveGradient();
    juce::Path path;
    
    g.setGradientFill (curveGradient);
    
    // Draw curve with NUM_POINTS points
    for (int i = 0; i < NUM_POINTS; ++i)
    {
        float t = static_cast<float> (i) / static_cast<float> (NUM_POINTS);
        
        float freq = frequencyAtTime (t);
        float ampl = curve.dbAtFrequency (freq);
        juce::Point<float> coords = coordsForFrequencyAndAmplitude (freq, ampl);
        if (i == 0)
            path.startNewSubPath (coords);
        else
            path.lineTo (coords);
    }
    g.strokePath (path, juce::PathStrokeType (CURVE_THICKNESS));
}

void CabinPeqGraph::drawDots (juce::Graphics& g)
{
    // Pseudocode
    
    // Get all bands in band profile
    
    // for each band, draw a corresponding point
    
    // add necessary exceptions for the dragging/hovering band id, the ghost node, etc.
    
    // reference CabinEqGraph::drawDots for more specific outline
    
    for (const auto& band : bands)
    {
        // Draw a dot corresponding to the node
        juce::Point<float> point = coordsForFrequencyAndAmplitude (band.freq, band.ampl);
        juce::Colour dotColour = getColourForFrequency (band.freq);
        
        // Figure out the radius - it's different if it's hovering vs. dragging
        float dotRadius = DOT_SIZE_DEFAULT;
        if (band.id == hoveringId || band.id == draggingId)
        {
            dotRadius = selectedDotSize;
        }
        
        drawDot (g, point, dotRadius, dotColour);
    }
    
    // Draw the ghost node for adding
    if (addingFreq.has_value())
    {
        // Figure out color of node
        juce::Colour addingDotColour = getColourForFrequency (addingFreq.value()).withAlpha (0.5f);
        
        // Calculate coordinates of node
        float addingAmpl = 0; // just put it directly on the line, for now
        juce::Point<float> point = coordsForFrequencyAndAmplitude (addingFreq.value(), addingAmpl);
        float addingDotRadius = DOT_SIZE_DEFAULT;
        
        // Draw node
        drawDot (g, point, addingDotRadius, addingDotColour);
    }
}

void CabinPeqGraph::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour)
{
    // Set the colour
    g.setColour (dotColour);
    
    // Draw dot
    g.fillEllipse (point.x - dotRadius - DOT_PADDING, point.y - dotRadius - DOT_PADDING, (dotRadius + DOT_PADDING) * 2, (dotRadius + DOT_PADDING) * 2);
    
    // Draw the center of the dot
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

void CabinPeqGraph::drawLines (juce::Graphics& g)
{
    // Draw the center line
    juce::Colour lineColour = juce::Colours::lightgrey;
    juce::PathStrokeType lineStrokeType (CURVE_THICKNESS / 2.0f);
    g.setColour (lineColour);
    
    juce::Path centerPath;
    float centerY = getY() + getHeight() / 2;
    centerPath.startNewSubPath (getX(), centerY);
    centerPath.lineTo (getX() + getWidth(), centerY);
    g.strokePath (centerPath, lineStrokeType);
    
    // Draw the log lines
    // draw lines starting at intervals of 10
    // every 10 it goes to intervals of 100
    // etc.
    
    std::vector<float> lineFreqs;
    float currLineFreq = 10;
    float interval = 10;
    while (currLineFreq <= 20000)
    {
        lineFreqs.push_back (currLineFreq);
        currLineFreq += interval;
        if (currLineFreq / interval >= 10)
            interval *= 10;
    }
    
    for (const auto& lineFreq : lineFreqs)
    {
        juce::Path logLinePath;
        float lineX = xForFreq (lineFreq);
        logLinePath.startNewSubPath (lineX, getY());
        logLinePath.lineTo (lineX, getY() + getHeight());
        g.strokePath (logLinePath, lineStrokeType);
    }
}

void CabinPeqGraph::updateHoveringStatus (const juce::MouseEvent& event)
{
    // Helpful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Show the ghost node to add if the mouse is on the center line
    if (std::abs (ampl) <= DIST_TO_ADD_DB)
        addingFreq = freq;
    else
        addingFreq.reset();
    
    // Figure out which node, if any, we're hovering over
    hoveringId = -1;
    auto hoveringBand = getClosestBandToMouseEvent (event);
    if (hoveringBand.has_value())
    {
        hoveringId = hoveringBand.value().id;
        startDragBandwidth = hoveringBand->bandwidth;
        selectedDotSize = DOT_SIZE_DRAGGING;
        
        // If we're hovering, we don't want to show the ghost node to add
        addingFreq.reset();
    }
}

juce::Colour CabinPeqGraph::getColourForFrequency (float frequency)
{
    if (isGrayscale)
        return juce::Colour::fromFloatRGBA (0.3f, 0.3, 0.3f, 1.0f);
    
    juce::Colour startColor;
    juce::Colour endColor;
    
    float t = (std::log2 (frequency) - std::log2 (MIN_FREQ)) / (std::log2 (MAX_FREQ) - std::log2 (MIN_FREQ));
    float segment_t;
    
    // Interpolate color from the start/end colors in each section
    if (t < 0.25f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 0.5f, 1.0f, 1.0f); // Deep blue
        endColor = juce::Colour::fromFloatRGBA(0.0f, 0.75f, 1.0f, 1.0f); // Sky blue
        segment_t = t / 0.25f;
    }
    else if (t < 0.5f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 0.75f, 1.0f, 1.0f); // Sky blue
        endColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.75f, 1.0f); // Light sea green
        segment_t = (t - 0.25f) / 0.25f;
    }
    else if (t < 0.75f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.75f, 1.0f); // Light sea green
        endColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.3f, 1.0f); // Spring green
        segment_t = (t - 0.5f) / 0.25f;
    }
    else
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.3f, 1.0f); // Spring green
        endColor = juce::Colour::fromFloatRGBA(0.7f, 1.0f, 0.3f, 1.0f); // Pastel yellow-green
        segment_t = (t - 0.75f) / 0.25f;
    }
    
    auto color = startColor.interpolatedWith (endColor, segment_t);
    return color;
}

juce::ColourGradient CabinPeqGraph::getCurveGradient()
{
    // Create initial gradient with start/end colors
    juce::Colour startColor = getColourForFrequency (minFreqShowing);
    juce::Colour endColor = getColourForFrequency (maxFreqShowing);
    juce::ColourGradient gradient (startColor, 0, 0, endColor, getWidth(), 0, false);
    
    // Useful helper to get color at specific point on screen
    float minFreqLog = std::log2 (minFreqShowing);
    float maxFreqLog = std::log2 (maxFreqShowing);
    auto calculateFreqLog = [minFreqLog, maxFreqLog](float factor) -> float
    {
        return minFreqLog + factor * (maxFreqLog - minFreqLog);
    };
    
    // Calculate colors at 25%, 50%, and 75%
    float quarterFreqLog = calculateFreqLog (0.25f);
    float halfFreqLog = calculateFreqLog (0.5f);
    float threeQuarterFreqLog = calculateFreqLog (0.75f);
    float quarterFreqX = (quarterFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();
    float halfFreqX = (halfFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();
    float threeQuarterFreqX = (threeQuarterFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();
    
    // Add the colors
    gradient.addColour (quarterFreqX / getWidth(), getColourForFrequency (std::pow (2, quarterFreqLog)));
    gradient.addColour (halfFreqX / getWidth(), getColourForFrequency (std::pow (2, halfFreqLog)));
    gradient.addColour (threeQuarterFreqX / getWidth(), getColourForFrequency (std::pow (2, threeQuarterFreqLog)));
    
    return gradient;
}

std::pair<float, float> CabinPeqGraph::getEventCoords (const juce::MouseEvent& event) const
{
    return { event.getPosition().getX() - getX(), event.getPosition().getY() - getY() };
}

juce::Point<float> CabinPeqGraph::coordsForFrequencyAndAmplitude (float freq, float ampl)
{
    // Calculate (x, y) coords and return
    float x = xForFreq (freq);
    float y = yForAmpl (ampl);
    
    return { x , y };
}

float CabinPeqGraph::xForFreq (float freq)
{
    return getWidth() * timeAtFrequency (freq) + getX();
}

float CabinPeqGraph::yForAmpl (float ampl)
{
    return getHeight() * (1.0f - (ampl - MIN_DB) / (MAX_DB - MIN_DB)); + getY();
}

std::pair<float, float> CabinPeqGraph::frequencyAndAmplitudeForCoords (float x, float y) const
{
    // Calculate frequency of x
    float freq = frequencyAtTime (x / getWidth());
    
    // Calculate amplitude of y
    float normalizedY = y / getHeight();
    float ampl = (1.0f - normalizedY) * (MAX_DB - MIN_DB) + MIN_DB;
    
    // Bound freq/ampl inside the visible window
    freq = std::max (std::min (freq, maxFreqShowing), minFreqShowing);
    ampl = std::min (std::max (ampl, MIN_DB), MAX_DB);
    
    return { freq, ampl };
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
    
    return frequencyAndAmplitudeForCoords (x, y);
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

std::optional<Band> CabinPeqGraph::getClosestBandToMouseEvent (const juce::MouseEvent& event) const
{
    float minDist = HOVER_MIN_DIST;
    std::optional<Band> closestBand;
    for (const auto& band : bands)
    {
        float dist = mouseEventDistanceFromBand (event, band);
        if (dist < minDist)
        {
            minDist = dist;
            closestBand = band;
        }
    }
    
    return closestBand;
}

int CabinPeqGraph::addBand(float freq, float ampl, float bandwidth)
{
    std::cout << "adding band" << std::endl;
    if (listener == nullptr)
        return -1;
    return listener->addBand(freq, ampl, bandwidth, this);
}

void CabinPeqGraph::updateBand (int id, float freq, float ampl, float bandwidth)
{
    if (listener != nullptr)
        listener->updateBand(id, freq, ampl, bandwidth, this);
}

void CabinPeqGraph::updateBandFromDrag (const juce::MouseEvent& event)
{
    auto currPos = getEventCoords (event);
    
    if (event.mods.isShiftDown())
    {
        dragOffsetWhileAdjustingBandwidth.first += currPos.first - lastDragPosition.first;
        dragOffsetWhileAdjustingBandwidth.second += currPos.second - lastDragPosition.second;
    }
    else
    {
        dragOffsetWhileAdjustingPosition.first += currPos.first - lastDragPosition.first;
        dragOffsetWhileAdjustingPosition.second += currPos.second - lastDragPosition.second;
    }
    
    // Update dragging node a final time
    float newX = startDragPosition.first + dragOffsetWhileAdjustingPosition.first;
    float newY = startDragPosition.second + dragOffsetWhileAdjustingPosition.second;
    auto [currFreq, currAmpl] = frequencyAndAmplitudeForCoords (newX, newY);
    float currBandwidth = startDragBandwidth / std::pow (1.2, dragOffsetWhileAdjustingBandwidth.second);
    
    updateBand (draggingId, currFreq, currAmpl, currBandwidth);
}

void CabinPeqGraph::removeBand(int id)
{
    if (listener != nullptr)
        listener->removeBand(id, this);
}

void CabinPeqGraph::startNoisePatternAt(int id)
{
    if (listener != nullptr)
    {
        listener->startNoisePatternAt(id, this);
    }
}

void CabinPeqGraph::updateNoisePatternAt(int id)
{
    if (listener != nullptr)
    {
        listener->updateNoisePatternAt(id, this);
    }
}

void CabinPeqGraph::stopNoisePattern()
{
    if (listener != nullptr)
    {
        listener->stopNoisePattern();
    }
}

void CabinPeqGraph::setNoisePatternSolo(bool solo)
{
    if (listener != nullptr)
    {
        listener->setNoisePatternSolo(solo);
    }
}

void CabinPeqGraph::soloNoisePatternIfAppropriate(const juce::MouseEvent& event)
{
    if (listener != nullptr)
    {
        listener->setNoisePatternSolo (event.mods.isCtrlDown() || event.mods.isAltDown());
    }
}
