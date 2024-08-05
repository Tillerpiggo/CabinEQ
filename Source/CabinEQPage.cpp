/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId)
    : processor (p), curveId (curveId)
{
    addAndMakeVisible (referenceSlider);
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    referenceSlider.addListener (this);
    
    updateEQNodes();
    startTimer (5);
}

CabinEQPage::~CabinEQPage()
{
    openGLContext.detach();
    referenceSlider.removeListener (this);
}

void CabinEQPage::paint (juce::Graphics& g)
{
    float currFreq = processor.getCurrPlayingFreq();
    g.fillAll (juce::Colour::fromFloatRGBA(0.04f, 0.04f, 0.04f, 1.0f));
    if (currFreq != 1000.0f)
        g.fillAll (getColorForFrequency (currFreq).withAlpha (0.2f));
    
    drawCurve (g, processor.getCurve (curveId), 4000);
    drawDots (g);
}

void CabinEQPage::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
    
    int sliderHeight = 50; // Set the height for the slider
    referenceSlider.setBounds (10, getHeight() - sliderHeight - 10, getWidth() - 20, sliderHeight);
}

void CabinEQPage::mouseMove (const juce::MouseEvent& event)
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        processor.startTestingAt (freq, curveId);
        return;
    }
    else
    {
        processor.endTesting();
    }
    
    isTestingFreq = false;
    
    hoveringId = -1;
    std::optional<EQNode> hoveringEQNode = getClosestEQNodeToMouseEvent (event);
    if (hoveringEQNode.has_value())
    {
        hoveringId = hoveringEQNode.value().id;
        if (draggingId == -1)
        {
            targetSelectedDotSize = DOT_SIZE_SELECTED;
        }
        else
        {
            targetSelectedDotSize = DOT_SIZE_DRAGGING;
        }
        
        if (addingFreq.has_value())
            addingFreq.reset();
    }
    else
    {
        if (dbDistanceFromCurve (freq, ampl) <= DIST_TO_ADD_DB)
        {
            addingFreq = freq;
        }
        else
        {
            addingFreq.reset();
        }
    }
    
//    repaint();
}

void CabinEQPage::mouseDown (const juce::MouseEvent& event)
{
    lastDistanceFromDragStartX = 0;
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.startTestingAt (freq, curveId);
        return;
    }
    isTestingFreq = false;
    
    std::optional<EQNode> draggingEQNode = getClosestEQNodeToMouseEvent (event);
    if (draggingEQNode.has_value())
    {
        draggingId = draggingEQNode.value().id;
        targetSelectedDotSize = DOT_SIZE_DRAGGING;
    }
    
    if (! draggingEQNode.has_value())
    {
        auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
        if (! event.mods.isRightButtonDown() && ampl > -24.0f && dbDistanceFromCurve (freq, ampl) < DIST_TO_ADD_DB)
        {
            draggingId = processor.addEQNode (freq, ampl, 0.0f, curveId);
            targetSelectedDotSize = DOT_SIZE_DRAGGING;
            addingFreq.reset();
        }
    }
    else
    {
        if (! event.mods.isRightButtonDown())
        {
            processor.startCalibratingEQNode (draggingEQNode.value());
        }
        else
        {
            processor.removeEQNode (draggingEQNode.value().id, curveId);
        }
        
        return;
    }
}

void CabinEQPage::mouseDrag (const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        isTestingFreq = true;
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.updateTestingAt (freq, curveId);
//        processor.updateSineSweep (freq);
//        processor.updateGreenNoise (freq);
        return;
    }
    isTestingFreq = false;
    
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    EQNode node (-1, 0, 0, 0);
    // Get the eq node in question
    for (const auto& eqNode : eqNodes)
        if (eqNode.id == draggingId)
            node = eqNode;
    
    node.frequency = freq;
    node.amplitude = ampl;
    
    processor.updateEQNode (draggingId, freq, ampl, node.pan, curveId);
    processor.updateCalibratingEQNode (node);
    
    // Drag white space if we aren't dragging a point
    if (draggingId == -1)
    {
        float tMinFreq = timeAtFrequency (minFreqShowing);
        float tMaxFreq = timeAtFrequency (maxFreqShowing);
        float t = (static_cast<float> (event.getDistanceFromDragStartX()) - lastDistanceFromDragStartX) / getWidth();
        lastDistanceFromDragStartX = static_cast<float> (event.getDistanceFromDragStartX());
        minFreqShowing = frequencyAtTime (tMinFreq - t);
        maxFreqShowing = frequencyAtTime (tMaxFreq - t);
    }
}

void CabinEQPage::mouseUp (const juce::MouseEvent& event)
{
    isTestingFreq = false;
    processor.endTesting();
    processor.endCalibratingEQNode();
    processor.endSineSweep();
    
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    EQNode node (0, 0, 0, 0);
    // Get the eq node in question
    for (const auto& eqNode : eqNodes)
        if (eqNode.id == draggingId)
            node = eqNode;
    
    node.frequency = freq;
    node.amplitude = ampl;
    
    processor.updateEQNode (draggingId, freq, ampl, node.pan, curveId);
    processor.endCalibratingEQNode();
    draggingId = -1;
    
    targetSelectedDotSize = DOT_SIZE_DEFAULT;
}

void CabinEQPage::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    isScrollingTimer = 3;
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
    float p = 1 - (wheel.deltaY);
    float dx = wheel.deltaX * -0.3f;
    
    float t = timeAtFrequency (freq);
    float leftChunkSize = t;
    float rightChunkSize = 1 - t;
    
    float leftSideOfWindow = t - (leftChunkSize * p) + dx;
    float rightSideOfWindow = t + (rightChunkSize * p) + dx;
    
    float projectedMinFreqVal = frequencyAtTime (leftSideOfWindow);
    float projectedMaxFreqVal = frequencyAtTime (rightSideOfWindow);
    
    if (projectedMinFreqVal < MIN_FREQ)
    {
//        float ratio = std::pow ((std::max (minFreqShowing, 0.0f) / MIN_FREQ), 4);
//        dx *= 0;//ratio;
//        //leftSideOfWindow = 0;//t - (leftChunkSize * p) + dx;
//        rightSideOfWindow = std::min (t + (rightChunkSize * p) + dx, MAX_FREQ);
        minFreqShowing = MIN_FREQ;//frequencyAtTime (leftSideOfWindow);
//        maxFreqShowing = frequencyAtTime (rightSideOfWindow);
    }
    if (projectedMaxFreqVal > MAX_FREQ)
    {
//        float ratio = std::pow ((MAX_FREQ / maxFreqShowing), 4);
//        dx *= 0;//ratio;
//        leftSideOfWindow = std::max (t - (leftChunkSize * p) + dx, MIN_FREQ);
//        //rightSideOfWindow = 1;//t + (rightChunkSize * p) + dx;
//        minFreqShowing = frequencyAtTime (leftSideOfWindow);
        maxFreqShowing = MAX_FREQ;//frequencyAtTime (rightSideOfWindow);
    }
    
    if (projectedMinFreqVal >= MIN_FREQ)
    {
        minFreqShowing = frequencyAtTime (leftSideOfWindow);
    }
    if (projectedMaxFreqVal <= MAX_FREQ)
    {
        maxFreqShowing = frequencyAtTime (rightSideOfWindow);
    }
}

void CabinEQPage::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &referenceSlider)
    {
        processor.setReferenceVolume (slider->getValue());
    }
}

void CabinEQPage::timerCallback()
{
    //rubberbandIfNotScrolling();
    repaint();
    
    if (isScrollingTimer > 1)
        isScrollingTimer--;
}

// ===================================================
void CabinEQPage::drawCurve(juce::Graphics& g, Curve& curve, int numPoints)
{
    float minFreqLog = std::log10(minFreqShowing);
    float maxFreqLog = std::log10(maxFreqShowing);

    // Get colors for min and max frequencies
    juce::Colour startColor = getColorForFrequency(minFreqShowing);
    juce::Colour endColor = getColorForFrequency(maxFreqShowing);

    // Define a gradient that transitions through the colors dynamically along the x-axis
    juce::ColourGradient gradient(
        startColor, // Color at minFreqShowing
        0, 0,
        endColor, // Color at maxFreqShowing
        getWidth(), 0, // Gradient is horizontal
        false
    );

    // Calculate x positions for intermediate frequencies based on logarithmic positions
    float quarterFreqLog = minFreqLog + 0.25f * (maxFreqLog - minFreqLog);
    float halfFreqLog = minFreqLog + 0.5f * (maxFreqLog - minFreqLog);
    float threeQuarterFreqLog = minFreqLog + 0.75f * (maxFreqLog - minFreqLog);

    float quarterFreqX = (quarterFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();
    float halfFreqX = (halfFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();
    float threeQuarterFreqX = (threeQuarterFreqLog - minFreqLog) / (maxFreqLog - minFreqLog) * getWidth();

    gradient.addColour(quarterFreqX / getWidth(), getColorForFrequency(std::pow(10, quarterFreqLog)));
    gradient.addColour(halfFreqX / getWidth(), getColorForFrequency(std::pow(10, halfFreqLog)));
    gradient.addColour(threeQuarterFreqX / getWidth(), getColorForFrequency(std::pow(10, threeQuarterFreqLog)));
    
//    if (draggingId != -1)
//    {
//        EQNode node (0, 0, 0, 0);
//        for (const auto& eqNode : eqNodes)
//            if (eqNode.id == draggingId)
//                node = eqNode;
//        float xVal = timeAtFrequency (node.frequency);
//        gradient.addColour (xVal, I_LIKE_THE_ORANGE);
//    }

    g.setGradientFill(gradient);

    juce::Path path;
    path.startNewSubPath(0, 0);
    
    int N = 300;
    
    for (int i = 0; i < N; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(N);
        
        float freq = frequencyAtTime(t);
        float ampl = juce::Decibels::gainToDecibels(curve.valueAtFrequency(freq).first.real());
        
        juce::Point<float> point = coordsForEQNode (freq, ampl);
        path.lineTo(coordsForEQNode(freq, ampl));
    }
    
    // Draw the main line
    g.strokePath(path, juce::PathStrokeType(CURVE_THICKNESS));
}

void CabinEQPage::updateEQNodes()
{
    eqNodes = processor.getEQNodes(curveId);
}

void CabinEQPage::updateSelectedDotSize()
{
    if (targetSelectedDotSize.has_value())
    {
        if (selectedDotSize < targetSelectedDotSize.value())
            selectedDotSize *= ANIM_STEP;
        else
            selectedDotSize /= ANIM_STEP;
        
        if (selectedDotSize > targetSelectedDotSize.value() / ANIM_STEP && selectedDotSize < targetSelectedDotSize.value() * ANIM_STEP)
        {
            if (targetSelectedDotSize.value() == DOT_SIZE_DEFAULT) 
            {
                hoveringId = -1;
            }
            selectedDotSize = targetSelectedDotSize.value();
            targetSelectedDotSize.reset();
        }
    }
}

void CabinEQPage::rubberbandIfNotScrolling()
{
    float t_minFreqShowing = timeAtFrequency (minFreqShowing);
    float t_maxFreqShowing = timeAtFrequency (maxFreqShowing);
    float t_minFreqShowingAfter = t_minFreqShowing;
    float t_maxFreqShowingAfter = t_maxFreqShowing;
    
    if (minFreqShowing < MIN_FREQ)
    {
        float t_MIN_FREQ = timeAtFrequency (MIN_FREQ);
        t_minFreqShowingAfter = (t_minFreqShowing + t_MIN_FREQ) / (2.0f);
        float dt = (t_minFreqShowing - t_minFreqShowingAfter);
        dt *= std::pow ((4 - isScrollingTimer) / 5.0f, 2.0f);
        t_minFreqShowingAfter = t_minFreqShowing - dt;
        t_maxFreqShowingAfter = t_maxFreqShowing - dt;
        minFreqShowing = frequencyAtTime (t_minFreqShowingAfter);
        maxFreqShowing = frequencyAtTime (t_maxFreqShowingAfter);
    }
    if (maxFreqShowing > MAX_FREQ)
    {
        float t_MAX_FREQ = timeAtFrequency (MAX_FREQ);
        t_maxFreqShowingAfter = (t_maxFreqShowing + t_MAX_FREQ) / (2.0f);
        float dt = t_maxFreqShowingAfter - t_maxFreqShowing;
        dt *= std::pow ((4 - isScrollingTimer) / 5.0f, 2.0f);
        t_minFreqShowingAfter = t_minFreqShowing + dt;
        t_maxFreqShowingAfter = t_maxFreqShowing + dt;
        minFreqShowing = frequencyAtTime (t_minFreqShowingAfter);
        maxFreqShowing = frequencyAtTime (t_maxFreqShowingAfter);
    }
    
    
}

juce::Colour CabinEQPage::getColorForFrequency(float frequency)
{
    float freqLogNorm = (std::log10(frequency) - std::log10(MIN_FREQ)) / (std::log10(MAX_FREQ) - std::log10(MIN_FREQ));
    juce::Colour startColor, endColor;
    float segmentLogNorm;

    if (freqLogNorm < 0.25f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 0.5f, 1.0f, 1.0f); // Deep blue
        endColor = juce::Colour::fromFloatRGBA(0.0f, 0.75f, 1.0f, 1.0f); // Sky blue
        segmentLogNorm = freqLogNorm / 0.25f;
    }
    else if (freqLogNorm < 0.5f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 0.75f, 1.0f, 1.0f); // Sky blue
        endColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.75f, 1.0f); // Light sea green
        segmentLogNorm = (freqLogNorm - 0.25f) / 0.25f;
    }
    else if (freqLogNorm < 0.75f)
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.75f, 1.0f); // Light sea green
        endColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.3f, 1.0f); // Spring green
        segmentLogNorm = (freqLogNorm - 0.5f) / 0.25f;
    }
    else
    {
        startColor = juce::Colour::fromFloatRGBA(0.0f, 1.0f, 0.3f, 1.0f); // Spring green
        endColor = juce::Colour::fromFloatRGBA(1.0f, 0.6f, 0.0f, 1.0f); // Warm orange
        segmentLogNorm = (freqLogNorm - 0.75f) / 0.25f;
    }

    return startColor.interpolatedWith(endColor, segmentLogNorm);
}

void CabinEQPage::drawDots (juce::Graphics& g)
{
    eqNodes = processor.getEQNodes(curveId);
    for (const auto& node : eqNodes)
    {
        float dbDifference = 0.0f;
        const auto& point = coordsForEQNode(node.frequency, node.amplitude - dbDifference);
        
        // Determine color based on frequency
        juce::Colour dotColor = getColorForFrequency(node.frequency);

        float dotRadius = 3.5f;
        if (node.id == hoveringId || node.id == draggingId)
        {
            dotRadius = selectedDotSize;
        }
            
        updateSelectedDotSize();
        
        float dotPadding = 3.0f;
        
        // Draw background color ellipse (assuming the background color is the same)
        g.setColour(backgroundColor);
        g.fillEllipse(point.x - dotRadius - dotPadding, point.y - dotRadius - dotPadding, (dotRadius + dotPadding) * 2, (dotRadius + dotPadding) * 2);
        
        //dotColor.withSaturation (245);// Bright orange for dragging

        // Draw the dot
        g.setColour(dotColor);
        g.fillEllipse(point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
        
        
    }
    
    // Draw the testing frequency dot
    if (isTestingFreq)
    {
        float freq = processor.getCurrTestingFreq();
        juce::Colour testDotColor = getColorForFrequency(freq).brighter(0.5f); // Make it brighter
        const auto& point = coordsForEQNode(freq, juce::Decibels::gainToDecibels(processor.getCurve(curveId).valueAtFrequency(freq).first.real()));
        float dotRadius = 5.0f;
        g.setColour(backgroundColor);
        g.fillEllipse(point.x - dotRadius - 2, point.y - dotRadius - 2, (dotRadius + 2) * 2, (dotRadius + 2) * 2);
        g.setColour(testDotColor);
        g.fillEllipse(point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
    }
    
    if (addingFreq.has_value())
    {
        juce::Colour addingDotColor = getColorForFrequency (addingFreq.value()).withAlpha (0.5f);
        const auto& point = coordsForEQNode(addingFreq.value(), juce::Decibels::gainToDecibels(processor.getCurve(curveId).valueAtFrequency(addingFreq.value()).first.real()));
        float dotRadius = DOT_SIZE_DEFAULT;
        g.setColour(backgroundColor);
        g.fillEllipse(point.x - dotRadius - 3, point.y - dotRadius - 3, (dotRadius + 3) * 2, (dotRadius + 3) * 2);
        g.setColour(addingDotColor);
        g.fillEllipse(point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
    }
}

juce::Point<float> CabinEQPage::coordsForEQNode (float frequency, float amplitude)
{
    float width = getWidth();
    float height = getHeight();
    
    amplitude -= -4.5 * std::log2 (frequency / 1000.0f);
    
    float x = width * timeAtFrequency (frequency);
    float y = height * (1.0f - (amplitude + 24.0f) / 48.0f);
    
    return { x, y };
}

std::pair<float, float> CabinEQPage::frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const
{
    float x = event.getPosition().x;
    float y = event.getPosition().y;
    
    // Calculate freq
    float freq = frequencyAtTime (x / getWidth());
    
    // Calculate amplitude
    float height = getHeight();
    float normalizedY = y / height;
    float ampl = (1.0f - normalizedY) * 48.0f - 24.0f;
    ampl += -4.5 * std::log2 (freq / 1000.0f);
    
    // Bound mouse events inside the visible window
    freq = std::max (std::min (freq, maxFreqShowing), minFreqShowing);
    ampl = std::min (std::max (ampl, -24.0f + -4.5f * std::log2 (freq / 1000.0f)), 24.0f + -4.5f * std::log2 (freq / 1000.0f));
    
    return { freq, ampl };
}

float CabinEQPage::frequencyAtTime (float t) const
{
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(minFreqShowing);
    float logMaxFreq = std::log(maxFreqShowing);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    return freq;
}

float CabinEQPage::timeAtFrequency (float freq) const
{
    float logMinFreq = std::log(minFreqShowing);
    float logMaxFreq = std::log(maxFreqShowing);
    float logFreq = std::log(freq);

    // Normalize the log frequency
    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
    return t;
}

bool CabinEQPage::mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const
{
    return mouseEventEQNodeDistance (event, eqNode) < 0.5f;
}

float CabinEQPage::mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    float xDist = (std::abs (timeAtFrequency (freq) - timeAtFrequency (eqNode.frequency))) * 39;
    float yDist = std::abs (ampl - eqNode.amplitude) * 0.5;
    float dist = std::sqrt (xDist * xDist + yDist * yDist);
    
    return dist;
}

float CabinEQPage::dbDistanceFromCurve (const float freq, const float ampl) const
{
    float curveGainAtFreq = processor.getCurve (curveId).valueAtFrequency (freq).first.real();
    float curveDBAtFreq = juce::Decibels::gainToDecibels (curveGainAtFreq);
    return std::abs (ampl - curveDBAtFreq);
}

std::optional<EQNode> CabinEQPage::getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const
{
    float minDist = 5.0f;
    std::optional<EQNode> closestEQNode;
    
    // Get id of node within distance range
    for (const auto& eqNode : eqNodes)
    {
        float dist = mouseEventEQNodeDistance (event, eqNode);
        if (mouseEventIsNearEQNode (event, eqNode) && dist < minDist)
        {
            minDist = dist;
            closestEQNode = eqNode;
        }
    }
    
    return closestEQNode;
}
