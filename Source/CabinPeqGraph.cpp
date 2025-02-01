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
    
    addAndMakeVisible (leftRightButton);
    addAndMakeVisible (instructionLabel);
    addAndMakeVisible (dimensionalSlider);
    dimensionalSlider.setListener (this);
    
    instructionLabel.setJustificationType (juce::Justification::bottomRight);
    instructionLabel.setInterceptsMouseClicks (false, true);
    
    addButton (&leftRightButton);
    addButtonAction (&leftRightButton, [this](juce::Button*) {
        int bandTypeInt = static_cast<int> (bandType);
        bandTypeInt = (bandTypeInt + 1) % 3;
        bandType = static_cast<Band::Type> (bandTypeInt);
        switch (bandType)
        {
            case Band::Type::both:
                leftRightButton.setButtonText ("BOTH");
                break;
            case Band::Type::left:
                leftRightButton.setButtonText ("LEFT");
                break;
            case Band::Type::right:
                leftRightButton.setButtonText ("RIGHT");
                break;
        }
    });
    
    // Initialize variables for faster painting
    instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
}

CabinPeqGraph::~CabinPeqGraph()
{
    removeListener();
}

void CabinPeqGraph::setBandProfile (BandProfile bandProfile)
{
    this->bandProfile = bandProfile;
    this->curve.updateWithBands (bandProfile.getBands());
    setVolume (bandProfile.getVolume());
}

void CabinPeqGraph::paint(juce::Graphics& g)
{
    // Draw your curve and log lines as usual
    drawLines(g);
    drawCurve(g);
    drawBands (g);
    drawDots (g);
}

void CabinPeqGraph::resized()
{
    setBounds (getBoundsInParent());
    
    // Add button in bottom right corner
//    leftRightButton.setBounds (getBounds().getWidth() - 100.0f, getBounds().getHeight() - 50.0f, 80.0f, 40.0f);
//    dimensionalSlider.setBounds (getBounds().getWidth() - 100.0f, getBounds().getHeight() - 100.0f, 100.0f, 100.0f);
    
    // Add instruction label in bottom right corner
    float instructionWidth = instructionLabel.getFont().getStringWidth (instructionLabel.getText());
    instructionLabel.setBounds (getBounds().getWidth() - instructionWidth, getBounds().getHeight() - 50.0f, instructionWidth, 40.0f);
    
    // Recalculate needed vars
    
    // horizontalLinePaths
    horizontalLinePaths.clear();
    int numHorizontalLines = 12;
    for (float y = 0; y <= getHeight(); y += getHeight() / numHorizontalLines)
    {
        juce::Path horizontalLinePath;
        horizontalLinePath.startNewSubPath (0, y);
        horizontalLinePath.lineTo (getWidth(), y);
        horizontalLinePaths.push_back (horizontalLinePath);
    }
    
    // centerPath
    float centerY = yForAmpl (0);
    centerPath.clear();
    centerPath.startNewSubPath (0, centerY);
    centerPath.lineTo (getWidth(), centerY);
    
    // lineFreqs
    lineFreqs.clear();
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
}

void CabinPeqGraph::mouseMove (const juce::MouseEvent &event)
{
    updateHoveringStatus (event);
    if (hoveringId != -1)
    {
        instructionLabel.setText (removeBandInstructions, juce::NotificationType::dontSendNotification);
    }
    else if (isHoveringOverDotControl)
    {
        instructionLabel.setText (volumeInstructions, juce::NotificationType::dontSendNotification);
    }
    else
    {
        instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
    }
    repaint();
}

void CabinPeqGraph::mouseDown (const juce::MouseEvent &event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    auto coords = getEventCoords (event);
    
    // Begin to track dragging
    lastDistanceFromDragStartX = 0;
    
    // If we aren't dragging/hovering, start a drag selection
    if (hoveringId == -1)
    {
        selectionStartFreq = freq;
        selectionEndFreq = freq;
    }
    
    // If we were hovering over a band node, we should now drag it
    draggingId = hoveringId;
    if (draggingId != -1)
    {
        // Let's set up selection, if applicable
        startDraggingBands.clear();
        // If we selected something, then set what we're dragging
        if (selectionStartFreq.has_value() && selectionEndFreq.has_value())
        {
            for (const auto& band : bandProfile.getBands())
            {
                if (band.freq >= selectionStartFreq.value() && band.freq <= selectionEndFreq.value())
                {
                    startDraggingBands.push_back (band);
                }
            }
        }
        else
        {
            startDraggingBands.clear();
        }
            
        updateBand (draggingId, freq, ampl, startDragBandwidth, bandType);
        
        selectedDotSize = DOT_SIZE_DRAGGING;
        startDragPosition = coords;
        lastDragPosition = coords;
        dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
        dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
        
        // If you drag something outside of the selection, reset the selection
        bool didSelectSelectedBand = false;
        for (const auto& band : startDraggingBands)
        {
            if (band.id == draggingId)
            {
                didSelectSelectedBand = true;
            }
        }
        
        if (! didSelectSelectedBand)
        {
            selectionStartFreq.reset();
            selectionEndFreq.reset();
            startDraggingBands.clear();
        }
    }
    
    // If we were going to add a band, do so here
    else if (addingFreq.has_value() && ! event.mods.isRightButtonDown() && ! isHoveringOverDotControl)
    {
        // Add the band where we click
        draggingId = addBand (freq, ampl, DEFAULT_BANDWIDTH, bandType);
        
        selectedDotSize = DOT_SIZE_DRAGGING;
        startDragPosition = coords;
        lastDragPosition = coords;
        startDragBandwidth = DEFAULT_BANDWIDTH;
        dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
        dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
        addingFreq.reset();
        
        selectionStartFreq.reset();
        selectionEndFreq.reset();
        startDraggingBands.clear();
    }
    
    // If we right click and were hovering, delete the band
    if (hoveringId != -1 && event.mods.isRightButtonDown())
    {
        removeBand (hoveringId);
        instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
    }
    
    // Update instruction text
    if (draggingId != -1 && ! event.mods.isRightButtonDown())
    {
        instructionLabel.setText (adjustBandwidthInstructions, juce::NotificationType::dontSendNotification);
    }
    
    repaint();
}

void CabinPeqGraph::mouseDrag (const juce::MouseEvent& event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    // If we're dragging the line, update profile amplitude
    if (isHoveringOverDotControl)
    {
        setVolume (ampl);
        repaint();
        return;
    }
    
    // If we're dragging a node, update it to our mouse position
    if (draggingId != -1)
    {
        updateBandFromDrag (event);
        
        lastDragPosition = getEventCoords (event);
    }
    
//    // If we're not dragging a node, we're dragging in the blackspace and should drag the graph itself
//    else
//    {
//        float minFreqShowingTime = timeAtFrequency (minFreqShowing);
//        float maxFreqShowingTime = timeAtFrequency (maxFreqShowing);
//        float timeChange = (static_cast<float> (event.getDistanceFromDragStart()) / -lastDistanceFromDragStartX) / getWidth();
//        
//        float projectedMinFreqShowing = frequencyAtTime (minFreqShowingTime - timeChange);
//        float projectedMaxFreqShowing = frequencyAtTime (maxFreqShowingTime - timeChange);
//        
//        // Apply changes if we are within the bounds of the graph
//        if (projectedMinFreqShowing >= MIN_FREQ && projectedMaxFreqShowing <= MAX_FREQ)
//        {
//            minFreqShowing = projectedMinFreqShowing;
//            maxFreqShowing = projectedMaxFreqShowing;
//        }
//    }
    
    // If we're not dragging anything, go select stuff
    if (draggingId == -1)
    {
        selectionEndFreq = freq;
        
        if (selectionEndFreq.value() < selectionStartFreq.value())
        {
            float temp = selectionEndFreq.value();
            selectionEndFreq = selectionStartFreq.value();
            selectionStartFreq = temp;
        }
    }
    
    repaint();
}

void CabinPeqGraph::mouseUp (const juce::MouseEvent& event)
{
    if (draggingId != -1)
    {
        updateBandFromDrag (event);
        draggingId = -1;
    }
    
    dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
    dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
    
    // Change the dot size back to normal
    selectedDotSize = DOT_SIZE_DEFAULT;
    
    instructionLabel.setText(addBandInstructions, juce::NotificationType::dontSendNotification);
    
    repaint();
}

void CabinPeqGraph::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    // Useful constants
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
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
    
    instructionLabel.setText (scrollInstructions, juce::NotificationType::dontSendNotification);
    
    repaint();
}

void CabinPeqGraph::positionChanged (juce::Point<float> pos)
{
    std::cout << "pos.x: " << pos.x << ", pos.y: " << pos.y << std::endl;
    auto [freq1, freq2] = dimensionalSliderPosToFreqs (pos);
    float bandwidth1 = (std::sin (std::log2 (freq2 / freq1)) + 1.5f);
    float bandwidth2 = (std::cos (std::log2 (freq1 / freq2)) + 1.5f);
    
    // Get the bands we're changing and update them
    for (const auto& band : bandProfile.getBands())
    {
        if (band.id == 0)
        {
            listener->updateBand (band.id, freq1, band.ampl, bandwidth1, band.type, currStepId);
        }
        if (band.id == 1)
        {
            listener->updateBand (band.id, freq2, band.ampl, bandwidth2, band.type, currStepId);
        }
    }
    
    updateBands();
}

void CabinPeqGraph::setProvisionalBands (std::vector<Band> provisionalBands)
{
    this->provisionalBands = provisionalBands;
}

void CabinPeqGraph::setProvisionalBandsVisible (bool provisionalBandsVisible)
{
    this->provisionalBandsVisible = provisionalBandsVisible;
}

void CabinPeqGraph::updateBands()
{
    if (dataSource != nullptr)
    {
        bandProfile = dataSource->getBandProfile();
        curve.updateWithBands (bandProfile.getBands());
        repaint();
    }
}

void CabinPeqGraph::timerCallback()
{
    repaint();
}

void CabinPeqGraph::setListener (CabinPeqGraphListener* listener)
{
    this->listener = listener;
}

void CabinPeqGraph::removeListener()
{
    this->listener = nullptr;
}

void CabinPeqGraph::addDataSource (CabinPeqGraphDataSource* dataSource)
{
    this->dataSource = dataSource;
}

void CabinPeqGraph::removeDataSource()
{
    this->dataSource = nullptr;
}

void CabinPeqGraph::setGrayscale (bool isGrayscale)
{
    this->isGrayscale = isGrayscale;
}

// =============================================
void CabinPeqGraph::drawLines (juce::Graphics& g)
{
    // Draw the center line
    g.setColour (centerLineColour);
    g.strokePath (centerPath, juce::PathStrokeType (CURVE_THICKNESS * 1.5f));
    
    // Draw the other horizontal lines
    g.setColour (lineColour);
    for (const auto& horizontalLinePath : horizontalLinePaths)
    {
        g.strokePath (horizontalLinePath, lineStrokeType);
    }
    
    // Draw the log lines
    // draw lines starting at intervals of 10
    // every 10 it goes to intervals of 100
    // etc.
    for (const auto& lineFreq : lineFreqs)
    {
        if (lineFreq >= minFreqShowing / 1.1f && lineFreq <= maxFreqShowing * 1.1f)
        {
            juce::Path logLinePath;
            float lineX = xForFreq (lineFreq);
            logLinePath.startNewSubPath (lineX, 0);
            logLinePath.lineTo (lineX, getHeight());
            g.strokePath (logLinePath, lineStrokeType);
        }
    }
}

void CabinPeqGraph::drawBands (juce::Graphics& g)
{
    for (const auto& band : bandProfile.getBands())
    {
        float bandAlpha = 0.3f;
        if (band.id == draggingId || band.id == hoveringId)
            bandAlpha = 0.8f;
        juce::Colour bandColour = getColourForFrequency (band.freq).withAlpha (bandAlpha);
        if (selectionStartFreq.has_value() && selectionEndFreq.has_value())
        {
            if (band.freq >= selectionStartFreq.value() && band.freq <= selectionEndFreq.value())
            {
                bandColour = juce::Colours::white.withAlpha (0.8f);
            }
        }
        
        if (band.type == Band::Type::left)
            bandColour = juce::Colours::red;
        if (band.type == Band::Type::right)
            bandColour = juce::Colours::purple;
        drawBand (g, band, bandColour);
//        // Get the color for the band
        
        
//        juce::Path path;
//        
//        if (band.type == Band::Type::left)
//            bandColour = juce::Colours::red;
//        if (band.type == Band::Type::right)
//            bandColour = juce::Colours::purple;
//        
//        // Draw curve with NUM_POINTS points
//        for (int i = 0; i < NUM_POINTS; ++i)
//        {
//            float t = static_cast<float> (i) / static_cast<float> (NUM_POINTS);
//            
//            float freq = frequencyAtTime (t);
//            float ampl = curve.dbAtFrequencyForBand (band, freq);
//            juce::Point<float> coords = coordsForFrequencyAndAmplitude (freq, ampl);
//            if (i == 0)
//            {
//                path.startNewSubPath (coords);
//            }
//            else
//            {
//                path.lineTo (coords);
//            }
//        }
//        
//        // Complete the shape and fill in with band color
//        path.lineTo (juce::Point<float> (getWidth(), yForAmpl (0)));
//        path.lineTo (juce::Point<float> (0, yForAmpl (0)));
//        g.setGradientFill (juce::ColourGradient (bandColour.withAlpha (bandAlpha), 0, 0, bandColour.withAlpha (bandAlpha), 0, getHeight(), false));
//        g.fillPath (path);
    }
    
    if (provisionalBandsVisible)
    {
        for (const auto& provisionalBand : provisionalBands)
        {
            drawBand (g, provisionalBand, juce::Colours::grey);
        }
    }
}

void CabinPeqGraph::drawBand (juce::Graphics& g, const Band& band, juce::Colour colour)
{
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
    path.lineTo (juce::Point<float> (getWidth(), yForAmpl (0)));
    path.lineTo (juce::Point<float> (0, yForAmpl (0)));
    g.setColour (colour);
    g.fillPath (path);
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
    
    juce::Path rectPath;
    rectPath.startNewSubPath (0, yForAmpl (0));
    rectPath.lineTo (0, yForAmpl (bandProfile.getVolume()));
    rectPath.lineTo (getWidth(), yForAmpl (bandProfile.getVolume()));
    rectPath.lineTo (getWidth(), yForAmpl (0));
    rectPath.lineTo (0, yForAmpl (0));
    g.setColour (juce::Colours::lightgrey.withAlpha (0.3f));
    g.fillPath (rectPath);
}

void CabinPeqGraph::drawDots (juce::Graphics& g)
{
    
    for (const auto& band : bandProfile.getBands())
    {
        // Draw a dot corresponding to the node
        juce::Point<float> point = coordsForFrequencyAndAmplitude (band.freq, band.ampl);
        juce::Colour dotColour = getColourForFrequency (band.freq);
        if (selectionStartFreq.has_value() && selectionEndFreq.has_value())
        {
            if (band.freq >= selectionStartFreq.value() && band.freq <= selectionEndFreq.value())
            {
                dotColour = juce::Colours::white;
            }
        }
        
        // Figure out the radius - it's different if it's hovering vs. dragging
        float dotRadius = DOT_SIZE_DEFAULT;
        
        drawDot (g, point, dotRadius, dotColour, band.id == draggingId);
    }
    
    // Draw the ghost node for adding
    if (addingFreq.has_value() && ! isHoveringOverDotControl)
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
    float ampl = bandProfile.getVolume();
    juce::Point<float> point = coordsForFrequencyAndAmplitude (freq, ampl);
    drawDot (g, point, DOT_SIZE_DEFAULT, dotColour, isHoveringOverDotControl);
    
    // Draw a dot over every currently playing freq
    if (dataSource != nullptr)
    {
        auto playingFreqs = dataSource->getCurrPlayingFreqs();
        for (const auto& playingFreq : playingFreqs)
        {
            juce::Colour playingDotColour = getColourForFrequency (playingFreq);
            juce::Point<float> playingPoint = coordsForFrequencyAndAmplitude (playingFreq, 0);
            float dotRadius = DOT_SIZE_DEFAULT;
            drawDot (g, playingPoint, dotRadius, playingDotColour, false);
        }
    }
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
    const float visibleXStart = 0;
    const float visibleXEnd = getWidth();

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
    isHoveringOverDotControl = mouseEventDistanceFromFrequencyAndAmplitude (event, MAX_FREQ, bandProfile.getVolume()) < DIST_TO_ADD_DB;
    if (isHoveringOverDotControl)
    {
        hoveringId = -1;
        draggingId = -1;
    }
}

juce::Colour CabinPeqGraph::getColourForFrequency (float frequency)
{
    if (isGrayscale)
        return CURVE_GRAYSCALE_COLOR;
    
    juce::Colour startColor;
    juce::Colour endColor;
    
    float t = (std::log2 (frequency) - std::log2 (MIN_FREQ)) / (std::log2 (MAX_FREQ) - std::log2 (MIN_FREQ));
    float segment_t;
    
    // Interpolate color from the start/end colors in each section
    if (t < 0.25f)
    {
        startColor = CURVE_GRADIENT_COLOR_1;
        endColor = CURVE_GRADIENT_COLOR_2;
        segment_t = t / 0.25f;
    }
    else if (t < 0.5f)
    {
        startColor = CURVE_GRADIENT_COLOR_2;
        endColor = CURVE_GRADIENT_COLOR_3;
        segment_t = (t - 0.25f) / 0.25f;
    }
    else if (t < 0.75f)
    {
        startColor = CURVE_GRADIENT_COLOR_3;
        endColor = CURVE_GRADIENT_COLOR_4;
        segment_t = (t - 0.5f) / 0.25f;
    }
    else
    {
        startColor = CURVE_GRADIENT_COLOR_4;
        endColor = CURVE_GRADIENT_COLOR_5;
        segment_t = (t - 0.75f) / 0.25f;
    }
    
    auto color = startColor.interpolatedWith (endColor, segment_t);
    return color;
}

juce::ColourGradient CabinPeqGraph::getCurveGradient()
{
    float alpha = 1.0f;
    
    // Create initial gradient with start/end colors
    juce::Colour startColor = getColourForFrequency (minFreqShowing).withAlpha (alpha);
    juce::Colour endColor = getColourForFrequency (maxFreqShowing).withAlpha (alpha);
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
    gradient.addColour (quarterFreqX / getWidth(), getColourForFrequency (std::pow (2, quarterFreqLog)).withAlpha (alpha));
    gradient.addColour (halfFreqX / getWidth(), getColourForFrequency (std::pow (2, halfFreqLog)).withAlpha (alpha));
    gradient.addColour (threeQuarterFreqX / getWidth(), getColourForFrequency (std::pow (2, threeQuarterFreqLog)).withAlpha (alpha));
    
    return gradient;
}

std::pair<float, float> CabinPeqGraph::getEventCoords (const juce::MouseEvent& event) const
{
    return { event.getPosition().getX(), event.getPosition().getY() };
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
    return getWidth() * timeAtFrequency (freq);
}

float CabinPeqGraph::yForAmpl (float ampl)
{
    return getHeight() * (1.0f - (ampl - MIN_DB) / (MAX_DB - MIN_DB));
}

std::pair<float, float> CabinPeqGraph::frequencyAndAmplitudeForCoords (float x, float y) const
{
    float padding = DOT_SIZE_DEFAULT + DOT_PADDING;
    
    // First, bound x and y inside window
    float boundedX = std::max (std::min (x, getWidth() - padding), padding) / getWidth();
    float boundedY = std::max (std::min (y, getHeight() - padding), padding) / getHeight();
    
    // Calculate frequency of x
    float freq = frequencyAtTime (boundedX);
    
    // Calculate amplitude of y
    float ampl = (1.0f - boundedY) * (MAX_DB - MIN_DB) + MIN_DB;

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
    float x = event.getPosition().x;
    float y = event.getPosition().y;
    
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
    for (const auto& band : bandProfile.getMultiBandSteps()[currStepId].getBands())
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

#include <cmath>

std::pair<float, float> CabinPeqGraph::dimensionalSliderPosToFreqs(juce::Point<float> pos)
{
    // Map slider position [-1, 1] to [0, 1] range
    float normalizedX = (pos.x + 1.0f) * 0.5f;
    float normalizedY = (pos.y + 1.0f) * 0.5f;

    // Convert normalized range [0, 1] to frequency range [minFreq, maxFreq] logarithmically
    float freq1 = dimensionalMinFreq * std::pow (dimensionalMaxFreq / dimensionalMinFreq, normalizedX);
    float freq2 = dimensionalMinFreq * std::pow (dimensionalMaxFreq / dimensionalMinFreq, normalizedY);

    return { freq1, freq2 };
}

juce::Point<float> CabinPeqGraph::freqsToDimensionalSliderPos(std::pair<float, float> freqs)
{
    // Frequencies are in the range [20.0, 20000.0]
    auto [freq1, freq2] = freqs;

    // Convert frequencies to normalized range [0, 1] logarithmically
    float normalizedX = std::log (freq1 / dimensionalMinFreq) / std::log(dimensionalMaxFreq / dimensionalMinFreq);
    float normalizedY = std::log (freq2 / dimensionalMinFreq) / std::log(dimensionalMaxFreq / dimensionalMinFreq);

    // Convert normalized range [0, 1] to slider position range [-1, 1]
    float x = (normalizedX * 2.0f) - 1.0f;
    float y = (normalizedY * 2.0f) - 1.0f;

    return { x, y };
}
int CabinPeqGraph::addBand (float freq, float ampl, float bandwidth, Band::Type type)
{
    if (listener == nullptr || dataSource == nullptr) // don't add a band unless we can reflect that change
        return -1;
    
    int newBandId = listener->addBand (freq, ampl, bandwidth, type, currStepId);
    updateBands();
    
    return newBandId;
}

void CabinPeqGraph::updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    listener->updateBand (id, freq, ampl, bandwidth, type, currStepId);
    updateBands();
    
    // If a dimensional slider controlled band was selected, update the slider
    if (id == 0 || id == 1)
    {
        // Calculate band freqs
        float geometricMean = std::sqrt (dimensionalMinFreq * dimensionalMinFreq + dimensionalMaxFreq * dimensionalMaxFreq);
        float freq1 = std::sqrt (geometricMean);
        float freq2 = std::sqrt (geometricMean);
        for (const auto& band : bandProfile.getBands())
        {
            if (band.id == 0)
            {
                freq1 = band.freq;
            }
            if (band.id == 1)
            {
                freq2 = band.freq;
            }
        }
        
        // Update dimensional slider with those freqs
        auto pos = freqsToDimensionalSliderPos ({ freq1, freq2 });
        dimensionalSlider.setPosition (pos);
    }
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
    
    if (selectionStartFreq.has_value() && selectionEndFreq.has_value())
    {
        for (const auto& band : startDraggingBands)
        {
            auto startPos = coordsForFrequencyAndAmplitude (band.freq, band.ampl);
            
            float newX = startPos.x + dragOffsetWhileAdjustingPosition.first;
            float newY = startPos.y + dragOffsetWhileAdjustingPosition.second;
            auto [currFreq, currAmpl] = frequencyAndAmplitudeForCoords (newX, newY);
            float currBandwidth = band.bandwidth * std::pow (1.05, dragOffsetWhileAdjustingBandwidth.second);
            
            updateBand (band.id, currFreq, currAmpl, currBandwidth, band.type);
        }
    }
    else
    {
        // Update dragging node a final time
        float newX = startDragPosition.first + dragOffsetWhileAdjustingPosition.first;
        float newY = startDragPosition.second + dragOffsetWhileAdjustingPosition.second;
        auto [currFreq, currAmpl] = frequencyAndAmplitudeForCoords (newX, newY);
        float currBandwidth = startDragBandwidth * std::pow (1.05, dragOffsetWhileAdjustingBandwidth.second);
        
        updateBand (draggingId, currFreq, currAmpl, currBandwidth, bandType);
    }
}

void CabinPeqGraph::removeBand (int id)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    listener->removeBand (id, currStepId);
    updateBands();
}

void CabinPeqGraph::setVolume (float volume)
{
    if (listener == nullptr || dataSource == nullptr)
        return;
    
    listener->setProfileVolume (volume);
    updateBands();
}
