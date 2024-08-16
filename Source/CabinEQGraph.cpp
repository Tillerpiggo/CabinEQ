/*
  ==============================================================================

    CabinEQGraph.cpp
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQGraph.h"

CabinEQGraph::CabinEQGraph()
{
    updateEQNodes();
    startTimer (5);
}

CabinEQGraph::~CabinEQGraph()
{
    removeListener();
}

void CabinEQGraph::setCurve (Curve& curve)
{
    this->curve = curve;
}

void CabinEQGraph::paint (juce::Graphics& g)
{
    g.fillAll (BACKGROUND_COLOR);
    
    if (curve.has_value())
    {
        drawCurve (g, curve->get(), 300);
        drawDots (g, curve->get());
    }
    
}

void CabinEQGraph::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CabinEQGraph::mouseMove (const juce::MouseEvent &event)
{
    cyclesSinceUserStoppedDoingShit = 0;
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // If ctrl/alt is held down, start testing
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        testValueAt (freq);
        hoveringId = -1;
        addingFreq.reset();
        return;
    }
    else 
    {
        // Since ctrl/alt isn't held down, stop testing
        isTestingFreq = false;
        stopTesting();
    }
    
    updateHoveringAndAddingNode (event);
}

void CabinEQGraph::mouseDown (const juce::MouseEvent &event)
{
    cyclesSinceUserStoppedDoingShit = 0;
    
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
        draggingId = addNode (freq, ampl);
        targetSelectedDotSize = DOT_SIZE_DRAGGING;
        addingFreq.reset();
        std::cout << "add node with id: " << draggingId << std::endl;
    }
    
    // If we right click and are hovering, delete the node
    if (hoveringId != -1 && event.mods.isRightButtonDown())
        removeNode (hoveringId);
        
    // If we ended up dragging a node, start playing tones
    if (draggingId != -1)
        startPlayingValueAt (freq, ampl);
}

void CabinEQGraph::mouseDrag (const juce::MouseEvent &event)
{
    cyclesSinceUserStoppedDoingShit = 0;
    
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // So that testing keeps moving when clicking
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        testValueAt (freq);
        hoveringId = -1;
        addingFreq.reset();
        return;
    }
    
    // If we're dragging a node, update it to our mouse position
    if (draggingId != -1)
    {
        updateNode (draggingId, freq, ampl);
        playValueAt (freq, ampl);
    }
    
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
    cyclesSinceUserStoppedDoingShit = 0;
    
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Update dragging node a final time
    updateNode (draggingId, freq, ampl);
    draggingId = -1;
    
    // Change the dot size
    targetSelectedDotSize = DOT_SIZE_DEFAULT;
    
    // We stopped dragging, so stop playing tones
    stopPlaying();
    stopTesting();
}

void CabinEQGraph::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    cyclesSinceUserStoppedDoingShit = 0;
    
    // Useful constants
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
    // So that testing keeps moving when clicking
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        testValueAt (freq);
        hoveringId = -1;
        addingFreq.reset();
    }
    
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
    
    updateHoveringAndAddingNode (event);
}

bool CabinEQGraph::keyPressed (const juce::KeyPress &key, juce::Component *originatingComponent)
{
    cyclesSinceUserStoppedDoingShit = 0;
    return true;
}

bool CabinEQGraph::keyStateChanged (bool isKeyDown, juce::Component *originatingComponent)
{
    cyclesSinceUserStoppedDoingShit = 0;
    isTestingFreq = false;
    stopTesting();
    return true;
}

void CabinEQGraph::timerCallback()
{
    updateSelectedDotSize();
    repaint();
    cyclesSinceUserStoppedDoingShit++;
    if (cyclesSinceUserStoppedDoingShit == 200)
        userStoppedDoingShit();
}

void CabinEQGraph::addListener (Listener* listener)
{
    this->listener = listener;
}

void CabinEQGraph::removeListener()
{
    this->listener = nullptr;
}

void CabinEQGraph::setGrayscale (bool grayscale)
{
    this->grayscale = grayscale;
}

// =============================================
void CabinEQGraph::drawCurve (juce::Graphics& g, Curve& curve, int numPoints)
{
    // Get the gradient for the curve
    juce::ColourGradient gradient = getCurveGradient();
    juce::Path path;
    
    g.setGradientFill (gradient);
    
    // Draw curve with numPoints points
    for (int i = 0; i < numPoints; ++i)
    {
        float t = static_cast<float> (i) / static_cast<float> (numPoints);
        
        float freq = frequencyAtTime (t);
        float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
        juce::Point<float> coords = coordsForEQNode (freq, ampl);
        if (i == 0)
            path.startNewSubPath (coords);
        else
            path.lineTo (coords);
    }
    g.strokePath (path, juce::PathStrokeType (CURVE_THICKNESS));
}

void CabinEQGraph::drawDots (juce::Graphics& g, Curve& curve)
{
    updateEQNodes();
    for (const auto& node : eqNodes)
    {
        // Draw a dot corresponding to the node
        juce::Point<float> point = coordsForEQNode (node.frequency, node.amplitude);
        juce::Colour dotColor = getColorForFrequency (node.frequency);
        
        // Figure out the radius - it's different if it's hovering vs. dragging
        float dotRadius = DOT_SIZE_DEFAULT;
        if (node.id == hoveringId || node.id == draggingId)
        {
            dotRadius = selectedDotSize;
            
            // We also need to change the selected dot size to sync with the reference tone playing
            if (getCurrPlayingFreq() != REFERENCE_FREQ)
            {
                dotColor = dotColor.interpolatedWith (juce::Colours::orange, 0.4);
                targetSelectedDotSize = DOT_SIZE_DRAGGING * 0.9;
            }
            else
            {
                targetSelectedDotSize = DOT_SIZE_DRAGGING;
            }
        }
        
        drawDot (g, point, dotRadius, dotColor);
    }
    
    // Draw the frequency testing dot
    if (isTestingFreq)
    {
        // Figure out color of node
        juce::Colour testDotColor = getColorForFrequency (getCurrPlayingFreq()).interpolatedWith (juce::Colours::pink, 0.4f);
        
        // Make node pulse w/ tones
        if (getCurrPlayingFreq() != REFERENCE_FREQ)
        {
            testDotColor = testDotColor.interpolatedWith (juce::Colours::orange, 0.4);
            targetSelectedDotSize = DOT_SIZE_DRAGGING * 0.9;
        }
        else
        {
            targetSelectedDotSize = DOT_SIZE_DRAGGING;
        }
        
        // Calculate coordinates of node
        float testingFreq = getCurrTestingFreq();
        juce::Point<float> point = coordsForEQNode (testingFreq, juce::Decibels::gainToDecibels (curve.valueAtFrequency (testingFreq).first.real()));
        
        // Draw node
        drawDot (g, point, selectedDotSize, testDotColor);
        
    }
    
    // Draw the ghost node for adding
    if (addingFreq.has_value())
    {
        // Figure out color of node
        juce::Colour addingDotColor = getColorForFrequency (addingFreq.value()).withAlpha (0.5f);
        
        // Calculate coordinates of node
        float addingAmpl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (addingFreq.value()).first.real());
        juce::Point<float> point = coordsForEQNode (addingFreq.value(), addingAmpl);
        float addingDotRadius = DOT_SIZE_DEFAULT;
        
        // Draw node
        drawDot (g, point, addingDotRadius, addingDotColor);
    }
}

void CabinEQGraph::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColor)
{
    // Draw padding around dot w/ background color
    g.setColour (BACKGROUND_COLOR);
    g.fillEllipse (point.x - dotRadius - DOT_PADDING, point.y - dotRadius - DOT_PADDING, (dotRadius + DOT_PADDING) * 2, (dotRadius + DOT_PADDING) * 2);
    
    // Draw the center of the dot
    g.setColour (dotColor);
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

void CabinEQGraph::updateSelectedDotSize()
{
    // Only update if there is currently a target selected dot size
    if (! targetSelectedDotSize.has_value())
        return;
    
    // If we're close enough to the target size, just become the target size and stop updating
    if (selectedDotSize > targetSelectedDotSize.value() / ANIM_STEP &&
        selectedDotSize < targetSelectedDotSize.value() * ANIM_STEP)
    {
        if (targetSelectedDotSize.value() == DOT_SIZE_DEFAULT)
            hoveringId = -1; // tbh not sure what this does, should see what happens if it's removed
        
        selectedDotSize = targetSelectedDotSize.value();
        targetSelectedDotSize.reset();
        return;
    }
    
    // Increment size towards target size
    if (selectedDotSize < targetSelectedDotSize.value())
        selectedDotSize *= ANIM_STEP;
    else
        selectedDotSize /= ANIM_STEP;
}

void CabinEQGraph::updateHoveringAndAddingNode (const juce::MouseEvent& event)
{
    if (! curve.has_value())
        return;
    
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // Show the ghost node to add if the mouse is on the curve
    if (dbDistanceFromCurve (freq, ampl, curve->get()) <= DIST_TO_ADD_DB)
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
    else if (! (event.mods.isCtrlDown() || event.mods.isAltDown()))
    {
        targetSelectedDotSize = DOT_SIZE_DEFAULT;
    }
}

juce::ColourGradient CabinEQGraph::getCurveGradient()
{
    // Create initial gradient with start/end colors
    juce::Colour startColor = getColorForFrequency (minFreqShowing);
    juce::Colour endColor = getColorForFrequency (maxFreqShowing);
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
    gradient.addColour (quarterFreqX / getWidth(), getColorForFrequency (std::pow (2, quarterFreqLog)));
    gradient.addColour (halfFreqX / getWidth(), getColorForFrequency (std::pow (2, halfFreqLog)));
    gradient.addColour (threeQuarterFreqX / getWidth(), getColorForFrequency (std::pow (2, threeQuarterFreqLog)));
    
    return gradient;
}

juce::Colour CabinEQGraph::getColorForFrequency (float frequency)
{
    if (grayscale)
        return juce::Colour::fromFloatRGBA (0.3f, 0.3f, 0.3f, 1.0f);
    
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
    
    return startColor.interpolatedWith (endColor, segment_t);
}

juce::Point<float> CabinEQGraph::coordsForEQNode (float frequency, float amplitude)
{
    // Calculate (x, y) coords and return
    float x = getWidth() * timeAtFrequency (frequency);
    float y = getHeight() * (1.0f - (amplitude + 12.0f) / 48.0f);
    
    return { x, y };
}

// ====================================================
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
    float ampl = (1.0f - normalizedY) * 48.0f - 12.0f;
    
    // Bound freq/ampl inside the visible window
    freq = std::max (std::min (freq, maxFreqShowing), minFreqShowing);
    ampl = std::min (std::max (ampl, -12.0f), 36.0f);
    
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

float CabinEQGraph::dbDistanceFromCurve (const float freq, const float ampl, Curve& curve) const
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
    if (curve.has_value())
        eqNodes = curve->get().getEQNodes();
}

int CabinEQGraph::addNode (float freq, float ampl)
{
    if (listener == nullptr)
        return -1;
    return listener->addNode (freq, ampl);
}

void CabinEQGraph::updateNode (int id, float freq, float ampl)
{
    if (listener != nullptr)
        listener->updateNode (id, freq, ampl);
}

void CabinEQGraph::removeNode (int id)
{
    if (listener != nullptr)
        listener->removeNode (id);
}

void CabinEQGraph::startPlayingValueAt (float freq, float ampl)
{
    if (listener != nullptr)
        listener->startPlayingValueAt (freq, ampl);
}

void CabinEQGraph::playValueAt (float freq, float ampl)
{
    if (listener != nullptr)
        listener->playValueAt (freq, ampl);
}

void CabinEQGraph::testValueAt (float freq)
{
    if (listener != nullptr)
        listener->testValueAt (freq);
}

void CabinEQGraph::stopPlaying()
{
    if (listener != nullptr)
        listener->stopPlaying();
}

void CabinEQGraph::stopTesting()
{
    if (listener != nullptr)
        listener->stopTesting();
}

float CabinEQGraph::getCurrPlayingFreq()
{
    if (listener == nullptr)
        return REFERENCE_FREQ;
    return listener->getCurrPlayingFreq();
}

float CabinEQGraph::getCurrTestingFreq()
{
    if (listener == nullptr)
        return REFERENCE_FREQ;
    return listener->getCurrTestingFreq();
}

void CabinEQGraph::userStoppedDoingShit()
{
    if (listener != nullptr)
        listener->userStoppedDoingShit();
}
