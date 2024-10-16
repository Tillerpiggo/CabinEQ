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
    drawBands (g);
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
void CabinPeqGraph::drawLines (juce::Graphics& g)
{
    // Draw the center line
    juce::Colour centerLineColour = juce::Colours::lightgrey;
    juce::Colour lineColour = juce::Colours::lightgrey.withAlpha (0.3f);
    juce::PathStrokeType lineStrokeType (CURVE_THICKNESS / 2.0f);
    
    juce::Path centerPath;
    float centerY = getY() + getHeight() / 2;
    centerPath.startNewSubPath (getX(), centerY);
    centerPath.lineTo (getX() + getWidth(), centerY);
    g.setColour (centerLineColour);
    g.strokePath (centerPath, juce::PathStrokeType (CURVE_THICKNESS));
    
    // Draw the other horizontal lines
    g.setColour (lineColour);
    int numHorizontalLines = 12;
    for (float y = getY(); y <= getY() + getHeight(); y += getHeight() / numHorizontalLines)
    {
        juce::Path horizontalLinePath;
        horizontalLinePath.startNewSubPath (getX(), y);
        horizontalLinePath.lineTo (getX() + getWidth(), y);
        g.strokePath (horizontalLinePath, lineStrokeType);
    }
    
    // Draw the log lines
    // draw lines starting at intervals of 10
    // every 10 it goes to intervals of 100
    // etc.
    
    std::vector<float> lineFreqs;
    float startLineFreq = 10;
    float currLineFreq = 10;
    float interval = 10;
    float numLines = 10;//std::pow (10.0f, std::round (2.0f - std::log10 (maxFreqShowing / minFreqShowing)));
    while (currLineFreq <= 20000)
    {
        lineFreqs.push_back (currLineFreq);
        currLineFreq += interval;
        if ((currLineFreq - startLineFreq) / interval >= numLines)
            interval *= 10;
    }
    
//    // Add extra lines if the interval between the lines is too large
//    std::vector<float> inBetweenLineFreqs;
//    for (int i = 0; i < lineFreqs.size() - 1; ++i)
//    {
//        // If the interval is too visually large, add in between lines
//        if (xForFreq (lineFreqs[i + 1]) - xForFreq (lineFreqs[i]) > getWidth() / 3.0f)
//        {
//            // Add 10 in between lines
//            float subInterval = (lineFreqs[i + 1] - lineFreqs[i]) / 10.0f;
//            for (int j = lineFreqs[i] + subInterval; j < lineFreqs[i + 1]; j += subInterval)
//                inBetweenLineFreqs.push_back (j);
//        }
//    }
//
//    // Add more lines that cut the original in half until no visible interval is too visibly large
//    for (int i = 0; i < lineFreqs.size() - 1; ++i)
//    {
//        // If the interval is too visually large,
//    }
//
//    for (const auto& inBetweenLineFreq : inBetweenLineFreqs)
//    {
//        std::cout << "inbetweenLineFreq: " << inBetweenLineFreq << std::endl;
//        lineFreqs.push_back (inBetweenLineFreq);
//    }
    
    for (const auto& lineFreq : lineFreqs)
    {
        if (lineFreq >= minFreqShowing / 1.1f && lineFreq <= maxFreqShowing * 1.1f)
        {
            juce::Path logLinePath;
            float lineX = xForFreq (lineFreq);
            logLinePath.startNewSubPath (lineX, getY());
            logLinePath.lineTo (lineX, getY() + getHeight());
            g.strokePath (logLinePath, lineStrokeType);
        }
    }
}

void CabinPeqGraph::drawBands (juce::Graphics& g)
{
    for (const auto& band : bands)
    {
        // Get the color for the band
        juce::Colour bandColour = getColourForFrequency (band.freq).withAlpha (0.3f);
        if (band.id == draggingId || band.id == hoveringId)
            bandColour = bandColour.withAlpha (0.8f);
        juce::Path path;
        
        // Draw curve with NUM_POINTS points
        for (int i = 0; i < NUM_POINTS; ++i)
        {
            float t = static_cast<float> (i) / static_cast<float> (NUM_POINTS);
            
            float freq = frequencyAtTime (t);
            float ampl = curve.dbAtFrequencyForBand (band, freq);
            juce::Point<float> coords = coordsForFrequencyAndAmplitude (freq, ampl);
            if (i == 0)
            {
                path.startNewSubPath (coords);
            }
            else
            {
                path.lineTo (coords);
            }
        }
        
        // Complete the shape and fill in with band color
        path.lineTo (juce::Point<float> (getX() + getWidth(), getY() + getHeight() / 2.0f));
        path.lineTo (juce::Point<float> (getX(), getY() + getHeight() / 2.0f));
        g.setColour (bandColour);
        g.fillPath (path);
    }
}

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
//        if (band.id == hoveringId || band.id == draggingId)
//        {
//            dotRadius = selectedDotSize;
//        }
        
        drawDot (g, point, dotRadius, dotColour, band.id == draggingId);
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
        drawDot (g, point, addingDotRadius, addingDotColour, false);
    }
    
    // Draw a dot on the end to control the overall volume of the profile
    juce::Colour dotColour = juce::Colours::lightgrey;
    float freq = MAX_FREQ;
    float ampl = profileAmpl;
    juce::Point<float> point = coordsForFrequencyAndAmplitude (freq, ampl);
    drawDot (g, point, DOT_SIZE_DEFAULT, dotColour, isHoveringOverDotControl);
}

void CabinPeqGraph::drawDot (juce::Graphics& g, juce::Point<float> point, float dotRadius, juce::Colour dotColour, bool isSelected)
{
    // Draw dot outside
    g.setColour (dotColour.withAlpha (isSelected ? 1.0f : 0.5f));
    g.fillEllipse (point.x - dotRadius - DOT_PADDING, point.y - dotRadius - DOT_PADDING, (dotRadius + DOT_PADDING) * 2, (dotRadius + DOT_PADDING) * 2);
    
    // Draw dot center
    g.setColour (dotColour.withAlpha (1.0f));
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
}

std::vector<float> CabinPeqGraph::getLogLines()
{
    std::vector<float> lineFreqs;
    float minFreq = minFreqShowing;
    float maxFreq = maxFreqShowing;
    const float minimalDistance = 50.0f; // Minimal distance in pixels between lines
    const float visibleXStart = getX();
    const float visibleXEnd = getX() + getWidth();

    // Step 1: Generate base frequencies
    int minDecade = static_cast<int>(std::floor(std::log10(minFreq)));
    int maxDecade = static_cast<int>(std::ceil(std::log10(maxFreq)));

    std::set<float> freqSet; // Use a set to keep frequencies unique and sorted
    for (int d = minDecade - 1; d <= maxDecade + 1; ++d)
    {
        float decadeBase = std::pow(10.0f, d);
        std::vector<float> multipliers = { 1.0f, 2.0f, 5.0f };
        for (float m : multipliers)
        {
            float f = m * decadeBase;
            // Include frequencies slightly outside the range to cover edges after mapping
            if (f >= minFreq * 0.8f && f <= maxFreq * 1.2f)
            {
                freqSet.insert(f);
            }
        }
    }

    // Step 2: Create initial intervals
    std::vector<float> frequencies(freqSet.begin(), freqSet.end());
    struct Interval
    {
        float f1;
        float f2;
    };
    std::list<Interval> intervalsToCheck;
    for (size_t i = 0; i + 1 < frequencies.size(); ++i)
    {
        intervalsToCheck.push_back({ frequencies[i], frequencies[i + 1] });
    }

    // Step 3: Subdivide intervals as needed
    while (!intervalsToCheck.empty())
    {
        auto it = intervalsToCheck.begin();
        while (it != intervalsToCheck.end())
        {
            float f1 = it->f1;
            float f2 = it->f2;
            // Map frequencies to x positions
            float x1 = xForFreq(f1);
            float x2 = xForFreq(f2);

            // Check if the interval is within or intersects the visible x range
            if ((x1 >= visibleXStart && x1 <= visibleXEnd) ||
                (x2 >= visibleXStart && x2 <= visibleXEnd) ||
                (x1 <= visibleXStart && x2 >= visibleXEnd) || // Interval spans visible area
                (x2 <= visibleXStart && x1 >= visibleXEnd))
            {
                float deltaX = std::abs(x2 - x1);
                if (deltaX > minimalDistance)
                {
                    // Subdivide interval
                    float f_mid = std::sqrt(f1 * f2); // Geometric mean for logarithmic scale
                    freqSet.insert(f_mid);
                    // Remove current interval and add new intervals
                    it = intervalsToCheck.erase(it);
                    intervalsToCheck.push_back({ f1, f_mid });
                    intervalsToCheck.push_back({ f_mid, f2 });
                    continue; // Continue without incrementing iterator
                }
            }
            // No subdivision needed, move to next interval
            ++it;
        }
    }

    // Convert set to vector
    lineFreqs = std::vector<float>(freqSet.begin(), freqSet.end());

    return lineFreqs;
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
    
    // Check if we're hovering over the dot control node at the end of the line
    isHoveringOverDotControl = mouseEventDistanceFromFrequencyAndAmplitude (event, MAX_FREQ, profileAmpl) < DIST_TO_ADD_DB;
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
    return getHeight() * (1.0f - (ampl - MIN_DB) / (MAX_DB - MIN_DB)) + getY();
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
    return mouseEventDistanceFromFrequencyAndAmplitude (event, band.freq, band.ampl);
}

float CabinPeqGraph::mouseEventDistanceFromFrequencyAndAmplitude (const juce::MouseEvent& event, float freq, float ampl) const
{
    // Calculate distance based on arbitrary scale factors that weight freq and ampl about the same
    // TODO: could theoretically improve the precision of this
    auto [mouseFreq, mouseAmpl] = frequencyAndAmplitudeForMouseEvent (event);
    float dx = std::abs (timeAtFrequency (mouseFreq) - timeAtFrequency (freq));
    float dy = std::abs (mouseAmpl - ampl);
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
