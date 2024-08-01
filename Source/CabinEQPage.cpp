/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId)
    : processor (p), curveId (curveId), curve (p.getCurve (curveId))
{
    addAndMakeVisible (referenceSlider);
    referenceSlider.setRange (-24.0f, 24.0f);
    referenceSlider.setValue (0.0f);
    referenceSlider.addListener (this);
    
    updateEQNodes();
    startTimer (10);
}

CabinEQPage::~CabinEQPage()
{
    referenceSlider.removeListener (this);
    stopTimer();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (34, 34, 34));
    drawCurve (g, curve, 4000);
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
    hoveringId = -1;
    
    std::optional<EQNode> hoveringEQNode = getClosestEQNodeToMouseEvent (event);
    if (hoveringEQNode.has_value())
        hoveringId = hoveringEQNode.value().id;
    
    repaint();
}

void CabinEQPage::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.startTestingAt (freq, curveId);
        
        return;
    }
    
    std::optional<EQNode> draggingEQNode = getClosestEQNodeToMouseEvent (event);
    if (draggingEQNode.has_value())
        draggingId = draggingEQNode.value().id;
    
    if (! draggingEQNode.has_value())
    {
        auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
        if (! event.mods.isRightButtonDown() && ampl > -24.0f)
            processor.addEQNode (freq, ampl, 0.0f, curveId);
        
        repaint();
    }
    else
    {
        if (! event.mods.isRightButtonDown())
        {
            processor.startCalibratingEQNode (draggingEQNode.value());
        }
        else
        {
            processor.removeEQNode(draggingEQNode.value().id, curveId);
        }
        
        repaint();
        return;
    }
}

void CabinEQPage::mouseDrag (const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        std::cout << "mouse drag!!" << std::endl;
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.updateTestingAt (freq, curveId);
//        processor.updateSineSweep (freq);
//        processor.updateGreenNoise (freq);
        
        return;
    }
    
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
    repaint();
}

void CabinEQPage::mouseUp (const juce::MouseEvent& event)
{
    processor.endTesting();
    processor.endCalibratingEQNode();
    processor.endSineSweep();
    processor.endGreenNoise();
    
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
    
    processor.applyCurve();
    repaint();
}

void CabinEQPage::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
    
    float p = 1 + (wheel.deltaY);
    float dx = wheel.deltaX * -0.3f;
    
    float t = timeAtFrequency (freq);
    float leftChunkSize = t;
    float rightChunkSize = 1 - t;
    
    float leftSideOfWindow = t - (leftChunkSize * p) + dx;
    float rightSideOfWindow = t + (rightChunkSize * p) + dx;
    
    
    minFreqShowing = frequencyAtTime (leftSideOfWindow);
    maxFreqShowing = frequencyAtTime (rightSideOfWindow);
    
    if (minFreqShowing < MIN_FREQ)
        minFreqShowing = MIN_FREQ;
    if (maxFreqShowing > MAX_FREQ)
        maxFreqShowing = MAX_FREQ;
    
    
    
//    if (wheel.deltaY > 0)
//    {
//        minFreqShowing = std::sqrt (freq * minFreqShowing);
//        maxFreqShowing = std::sqrt (freq * maxFreqShowing);
//    }
//    else
//    {
//        minFreqShowing = std::sqrt (MIN_FREQ * minFreqShowing);
//        maxFreqShowing = std::sqrt (MAX_FREQ * maxFreqShowing);
//    }
    repaint();
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
    repaint();
}


//==========================
void CabinEQPage::drawCurve (juce::Graphics& g, const Curve& curve, int numPoints)
{
    g.setColour (juce::Colour::fromRGB(0, 255, 128));

    juce::Path path;
    path.startNewSubPath(0, 0);
    
    int N = 4000;
    
    for (int i = 0; i < N; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(N);
        
        float freq = frequencyAtTime (t);
        float ampl = juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real());
        
        path.lineTo (coordsForEQNode (freq, ampl));
    }
    g.strokePath (path, juce::PathStrokeType (1.0f));
}

void CabinEQPage::updateEQNodes()
{
    eqNodes = processor.getEQNodes();
}

void CabinEQPage::drawDots (juce::Graphics& g)
{
    eqNodes = processor.getEQNodes();
    g.setColour (juce::Colour::fromRGB (255, 0, 255)); // Bright magenta
    
    for (const auto& node : eqNodes)
    {
        if (node.id == draggingId)
            g.setColour (juce::Colour::fromRGB (255, 255, 0));
        
//        float dbDifference = -4.5f * std::log2((node.frequency) / 1000.0f);
        float dbDifference = 0.0f;
        const auto& point = coordsForEQNode (node.frequency, node.amplitude - dbDifference);
        
        float dotRadius = 4.0f;
        if (node.id == hoveringId)
            dotRadius = 6.0f;
        g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
        
        if (node.id == draggingId)
            g.setColour (juce::Colour::fromRGB (255, 0, 255));
    }
    
    g.setColour (juce::Colour::fromRGB (0, 255, 255));
    
    float freq = processor.getCurrTestingFreq();
    const auto& point = coordsForEQNode (freq, juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real()));
    float dotRadius = 3.0f;
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
    
    
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
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    float freqRatio = freq / eqNode.frequency;
    float amplDiff = std::abs (ampl - eqNode.amplitude);
    
    return freqRatio > 0.9 && freqRatio < 1.1 && amplDiff < 2.0;
}

float CabinEQPage::mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    float freqRatio = freq / eqNode.frequency;
    
    return std::abs (1.0f - freqRatio);
}

std::optional<EQNode> CabinEQPage::getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const
{
    float minDist = 0.03f * (std::log (maxFreqShowing / minFreqShowing)) / 3.0f;
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
