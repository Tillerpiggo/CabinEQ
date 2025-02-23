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
    initializeLabels();
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
    drawNoise(g);
    drawCurve(g);
    drawBands (g);
    drawDots (g);
    drawSelection (g);
}

void CabinPeqGraph::resized()
{
    setBounds (getBoundsInParent());    

    // Hide instruction label for now
    // instructionLabel.setBounds (0.0f, getBounds().getHeight() - 50.0f, getWidth(), 40.0f);
    
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

    // Set bounds of freq/ampl labels

     // Distribute freq labels accordingly, based on the freqs
     std::vector<float> freqLabelsX;
     std::vector<float> freqs = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
     float offset = 0;
     for (int i = 0; i < freqs.size(); i++)
     {
        freqLabelsX.push_back (xForFreq (freqs[i]) + offset);
     }
     for (int i = 0; i < freqLabelsX.size(); i++)
     {
         float freqLabelWidth = freqLabels[i].getFont().getStringWidth (freqLabels[i].getText());
         freqLabels[i].setBounds (freqLabelsX[i] - freqLabelWidth / 2, getHeight() - 40, freqLabelWidth, 20);
     }

//    // Distribute ampl labels accordingly, matching with the horizontal lines
//    std::vector<float> amplLabelsY;
//    std::vector<float> amplitudes = { -30, -24, -18, -12, -6, 0, 6, 12, 18, 24, 30 };
//    for (int i = 0; i < lineFreqs.size(); i++)
//    {
//        amplLabelsY.push_back (yForAmpl (amplitudes[i]) + offset);
//    }
//    for (int i = 0; i < amplLabelsY.size(); i++)
//    {
//        float amplLabelWidth = amplLabels[i].getFont().getStringWidth (amplLabels[i].getText());
//        amplLabels[i].setBounds (0, amplLabelsY[i] - amplLabelWidth / 2, 20, amplLabelWidth);
//    }
}

void CabinPeqGraph::mouseMove (const juce::MouseEvent &event)
{
    updateHoveringStatus (event);
    if (hoveringId != -1)
    {
        if (selectedIds.size() == 0)
        {
            instructionLabel.setText (removeBandInstructions, juce::NotificationType::dontSendNotification);
        }
        else if (selectedIds.find (hoveringId) == selectedIds.end())
        {
            instructionLabel.setText (shiftClickBandInstructions, juce::NotificationType::dontSendNotification);
        }
        else
        {
            instructionLabel.setText (shiftClickRemoveBandInstructions, juce::NotificationType::dontSendNotification);
        }
    }
    else if (isHoveringOverDotControl)
    {
        instructionLabel.setText (volumeInstructions, juce::NotificationType::dontSendNotification);
    }
    else
    {
        if (selectedIds.size() == 0)
        {
            instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
        }
        else
        {
            instructionLabel.setText (groupDragInstructions, juce::NotificationType::dontSendNotification);
        }
    }
}

void CabinPeqGraph::mouseDown (const juce::MouseEvent &event)
{
    // Useful constants
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    auto coords = getEventCoords (event);
    
    // Begin to track dragging
    lastDistanceFromDragStartX = 0;
    
    // If we aren't dragging/hovering, start a drag selection
    if (hoveringId == -1 && ! addingFreq.has_value())
    {
        auto eventPos = event.getPosition().toFloat();
        selectionStart = eventPos;
        selectionEnd = eventPos;
        selectionRect = { eventPos, eventPos };
        selectedIds.clear();
        selectedIdToStartingValue.clear();
        instructionLabel.setText (selectInstructions, juce::NotificationType::dontSendNotification);
        
        return;
    }
    
    selectionStart.reset();
    selectionEnd.reset();
    selectionRect.reset();
    
    // If we were hovering over a band node, we should now drag it
    draggingId = hoveringId;
    if (draggingId != -1)
    {
        if (! event.mods.isShiftDown())
        {
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
        }
        
        // If shift clicking, just add/remove it from the group
        else
        {
            if (selectedIds.find (draggingId) == selectedIds.end())
            {
                selectedIds.insert (draggingId);
                std::optional<Band> draggingBand;
                for (const auto& band : bandProfile.getBands())
                {
                    if (band.id == draggingId)
                    {
                        draggingBand = band;
                        break;
                    }
                }
                
                if (draggingBand.has_value())
                    selectedIdToStartingValue[draggingId] = draggingBand.value();
            }
            else
            {
                selectedIds.erase (draggingId);
                selectedIdToStartingValue.erase (draggingId);
            }
            draggingId = -1;
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
        
        clearSelection();
    }
    
    // If we right click and were hovering, delete the band
    if (hoveringId != -1 && event.mods.isRightButtonDown())
    {
        removeBand (hoveringId);
        
        // If we selected multiple, remove all of them...
        if (selectedIds.find (hoveringId) != selectedIds.end())
        {
            for (const auto& id : selectedIds)
            {
                removeBand (id);
            }
            selectedIds.clear();
        }
        
        if (selectedIds.size() == 0)
        {
            instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
        }
        else
        {
            instructionLabel.setText (groupDragInstructions, juce::NotificationType::dontSendNotification);
        }
    }
    
    // Update instruction text
    if (draggingId != -1 && ! event.mods.isRightButtonDown())
    {
        if (selectedIds.size() == 0)
        {
            instructionLabel.setText (adjustBandwidthInstructions, juce::NotificationType::dontSendNotification);
        }
        else
        {
            instructionLabel.setText (groupAdjustBandwidthInstructions, juce::NotificationType::dontSendNotification);
        }
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
    
    // We're dragging a node
    if (draggingId != -1)
    {
        updateBandFromDrag (event);
        lastDragPosition = getEventCoords (event);
    }
    
    // If we're not dragging anything, go select stuff
    if (draggingId == -1 && selectionStart.has_value() && selectionEnd.has_value())
    {
        selectionEnd = event.getPosition().toFloat();
        
        // Get constants for selection
        float xMin = fmin (selectionStart->x, selectionEnd->x);
        float xMax = fmax (selectionStart->x, selectionEnd->x);
        float yMin = fmin (selectionStart->y, selectionEnd->y);
        float yMax = fmax (selectionStart->y, selectionEnd->y);
        selectionRect = { xMin, yMin, xMax - xMin, yMax - yMin };
        
        selectedIds.clear();
        for (const auto& band : bandProfile.getBands())
        {
            auto pos = getLocalCoordsForBand (band);
            if (selectionRect->contains (pos))
            {
                selectedIds.insert (band.id);
                selectedIdToStartingValue[band.id] = band;
            }
        }
    }
    
    repaint();
}

void CabinPeqGraph::mouseUp (const juce::MouseEvent& event)
{
    if (selectionStart.has_value() || selectionEnd.has_value())
    {
        selectionStart.reset();
        selectionEnd.reset();
        selectionRect.reset();
    }
    
    if (draggingId != -1)
    {
        updateBandFromDrag (event);
        
        // Update selected id values
        for (const auto& band : bandProfile.getBands())
        {
            if (selectedIds.find (band.id) != selectedIds.end())
            {
                selectedIdToStartingValue[band.id] = band;
            }
        }
        
        draggingId = -1;
    }
    
    dragOffsetWhileAdjustingPosition = { 0.0f, 0.0f };
    dragOffsetWhileAdjustingBandwidth = { 0.0f, 0.0f };
    
    // Change the dot size back to normal
    selectedDotSize = DOT_SIZE_DEFAULT;
    
    if (selectedIds.size() == 0)
    {
        instructionLabel.setText (addBandInstructions, juce::NotificationType::dontSendNotification);
    }
    else
    {
        instructionLabel.setText (groupDragInstructions, juce::NotificationType::dontSendNotification);
    }
    
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

bool CabinPeqGraph::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::deleteKey)
    {
        // Delete all selected bands
        for (const auto& id : selectedBandIds)
        {
            removeBand (id);
            clearSelection();
        }
        
        return true;
    }

    return false;
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
    g.setColour (CENTER_LINE_COLOUR);
    g.strokePath (centerPath, juce::PathStrokeType (CURVE_THICKNESS * 1.5f));
    
    // Draw the other horizontal lines
    g.setColour (LINE_COLOUR);
    for (const auto& horizontalLinePath : horizontalLinePaths)
    {
        g.strokePath (horizontalLinePath, lineStrokeType);
    }
    
    // Draw the log lines
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

void CabinPeqGraph::drawNoise (juce::Graphics& g)
{
    // Draw a faded rectangle with bandwidth around the current noise point
    
    // Draw a dot over every currently playing freq
    if (dataSource != nullptr)
    {
//        auto playingFreqs = dataSource->getCurrPlayingFreqs();
        auto playingFreqsAndVols = dataSource->getCurrPlayingFreqsAndVols();
        auto bandwidth = dataSource->getBandwidth();
        for (const auto& playingFreqAndVol : playingFreqsAndVols)
        {
            auto [playingFreq, playingVol] = playingFreqAndVol;
            float lowFreq = playingFreq / std::pow (2.0f, bandwidth);
            float highFreq = playingFreq * std::pow (2.0f, bandwidth);
            float startX = xForFreq (lowFreq);
            float endX = xForFreq (highFreq);
            juce::Colour startColour = getColourForFrequency (lowFreq).withAlpha (0.0f);
            juce::Colour  midColour = getColourForFrequency (playingFreq).withAlpha (0.3f * playingVol);
            juce::Colour endColour = getColourForFrequency (highFreq).withAlpha (0.0f);
            juce::ColourGradient gradient (startColour, startX, 0, endColour, endX, 0, false);
            gradient.addColour (0.5, midColour);
            
            g.setGradientFill (gradient);
            g.fillRect (startX, 0.0f, endX - startX, (float) getHeight());
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
        if (selectedIds.find (band.id) != selectedIds.end())
            bandColour = SELECTED_BAND_COLOUR;
        
        if (band.type == Band::Type::left)
            bandColour = juce::Colours::red;
        if (band.type == Band::Type::right)
            bandColour = juce::Colours::purple;
        drawBand (g, band, bandColour);
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
    g.setColour (VOLUME_RECT_COLOUR);
    g.fillPath (rectPath);
}

void CabinPeqGraph::drawDots (juce::Graphics& g)
{
    
    for (const auto& band : bandProfile.getBands())
    {
        // Draw a dot corresponding to the node
        juce::Point<float> point = coordsForFrequencyAndAmplitude (band.freq, band.ampl);
        juce::Colour dotColour = getColourForFrequency (band.freq);
        if (selectedIds.find (band.id) != selectedIds.end())
            dotColour = SELECTION_BORDER_COLOUR;
        
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
}

void CabinPeqGraph::drawSelection (juce::Graphics& g)
{
    if (selectionRect.has_value())
    {
        g.setColour (SELECTION_COLOUR);
        g.fillRect (selectionRect.value());
        g.setColour (SELECTION_BORDER_COLOUR);
        g.drawRect (selectionRect.value());
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

juce::Point<float> CabinPeqGraph::getLocalCoordsForBand (const Band band)
{
    return coordsForFrequencyAndAmplitude (band.freq, band.ampl);
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
    
    // If we're dragging multiple, must update multiple
    if (! selectedIds.empty())
    {
        Band draggedBand = selectedIdToStartingValue[draggingId];
        auto draggedStartPos = getLocalCoordsForBand (draggedBand);
        float draggedCurrY = draggedStartPos.y + dragOffsetWhileAdjustingPosition.second;
        auto [_, currAmpl] = frequencyAndAmplitudeForCoords (0, draggedCurrY);
        float amplFactor = currAmpl / draggedBand.ampl;
        for (const auto& bandId : selectedIds)
        {
            Band band = selectedIdToStartingValue[bandId];
            auto startPos = getLocalCoordsForBand (band);
            
            float newX = startPos.x + dragOffsetWhileAdjustingPosition.first;
            float newY = startPos.y + dragOffsetWhileAdjustingPosition.second;

            auto [currFreq, _] = frequencyAndAmplitudeForCoords (newX, 0);
//            float amplScaleFactor = currAmpl / band.ampl;
            float currBandwidth = std::min (band.bandwidth * std::pow (1.05f, dragOffsetWhileAdjustingBandwidth.second), 32.0f);
            
            float projAmpl = band.ampl * amplFactor;
            float maxAmpl = std::min (std::abs (projAmpl), MAX_DB);
            if (projAmpl < 0)
                maxAmpl *= -1.0f;
            
            updateBand (band.id, currFreq, projAmpl, currBandwidth, band.type);
        }
    }
    else
    {
        // Update dragging node a final time
        float newX = startDragPosition.first + dragOffsetWhileAdjustingPosition.first;
        float newY = startDragPosition.second + dragOffsetWhileAdjustingPosition.second;
        auto [currFreq, currAmpl] = frequencyAndAmplitudeForCoords (newX, newY);
        float currBandwidth = std::min (startDragBandwidth * std::pow (1.05f, dragOffsetWhileAdjustingBandwidth.second), 32.0f);
        
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

void CabinPeqGraph::initializeLabels()
{
    // // Logarithmically distribute frequency labels w/ 20, 50, 100, 200, 500, 1k, 2k, 5k, 10k, 20k
    // std::vector<std::string> freqLabelsText = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };
    // for (int i = 0; i < 10; i++)
    // {
    //     freqLabels[i].setText (freqLabelsText[i], juce::NotificationType::dontSendNotification);
    //     addAndMakeVisible (freqLabels[i]);
    // }

    // // Linearly distribute amplitude labels w/ -30, -24, -18, -12, -6, 0, 6, 12, 18, 24, 30
    // for (int i = -5; i <= 5; i++)
    // {
    //     amplLabels[i + 5].setText (std::to_string (i * 6), juce::NotificationType::dontSendNotification);
    //     addAndMakeVisible (amplLabels[i + 5]);
    // }
}

void CabinPeqGraph::clearSelection()
{
    selectionStart.reset();
    selectionEnd.reset();
    selectionRect.reset();
    selectedIds.clear();
    selectedIdToStartingValue.clear();
}
