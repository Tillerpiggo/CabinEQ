/*
  ==============================================================================

    CabinEQGraph.cpp
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#include "CabinEQGraph.h"

CabinEQGraph::CabinEQGraph (const Curve& curve)
    : curve (curve)
{
    updateEQNodes();
}

CabinEQGraph::~CabinEQGraph()
{
    delete listener;
}

void CabinEQGraph::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (34, 34, 34));
    drawCurve (g, curve, 4000);
    drawDots (g);
}

void CabinEQGraph::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CabinEQGraph::mouseMove (const juce::MouseEvent& event)
{
    hoveringId = -1;
    
    std::optional<EQNode> hoveringEQNode = getClosestEQNodeToMouseEvent (event);
    if (hoveringEQNode.has_value())
        hoveringId = hoveringEQNode.value().id;
    
    repaint();
    
}

void CabinEQGraph::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        listener->testValueAt (freq);
        return;
    }
    
    std::optional<EQNode> draggingEQNode = getClosestEQNodeToMouseEvent (event);
    if (draggingEQNode.has_value())
        draggingId = draggingEQNode.value().id;
    
    if (! draggingEQNode.has_value())
    {
        auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
        if (! event.mods.isRightButtonDown() && ampl > -24.0f)
            listener->nodeAdded (freq, ampl);
        
        repaint();
    }
    else
    {
        if (! event.mods.isRightButtonDown())
        {
            listener->playValueAt (draggingEQNode->frequency, draggingEQNode->amplitude);
        }
        else
        {
            listener->nodeRemoved (draggingEQNode->id);
        }
        
        repaint();
        return;
    }
}

void CabinEQGraph::mouseDrag (const juce::MouseEvent& event)
{
    if (event.mods.isCtrlDown() || event.mods.isAltDown())
    {
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        listener->testValueAt (freq);
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
    
    listener->nodeMoved (draggingId, freq, ampl);
    listener->playValueAt (node.frequency, node.amplitude);
    repaint();
}

void CabinEQGraph::mouseUp (const juce::MouseEvent& event)
{
    listener->stopPlaying();
    
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    EQNode node (0, 0, 0, 0);
    // Get the eq node in question
    for (const auto& eqNode : eqNodes)
        if (eqNode.id == draggingId)
            node = eqNode;
    
    node.frequency = freq;
    node.amplitude = ampl;
    
    listener->nodeMoved (draggingId, freq, ampl);
    listener->stopPlaying();
    draggingId = -1;
    repaint();
}

void CabinEQGraph::mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
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
    
    repaint();
}

//==========================
void CabinEQGraph::drawCurve (juce::Graphics& g, const Curve& curve, int numPoints)
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

void CabinEQGraph::addListener (Listener* newListener)
{
    this->listener = newListener;
}

// ============================================
void CabinEQGraph::updateEQNodes()
{
    eqNodes = listener->getEQNodes();
}

void CabinEQGraph::drawDots (juce::Graphics& g)
{
    updateEQNodes();
    g.setColour (juce::Colour::fromRGB (255, 0, 255)); // Bright magenta
    
    for (const auto& node : eqNodes)
    {
        if (node.id == draggingId)
            g.setColour (juce::Colour::fromRGB (255, 255, 0));
        
        const auto& point = coordsForEQNode (node.frequency, node.amplitude);
        
        float dotRadius = 4.0f;
        if (node.id == hoveringId)
            dotRadius = 6.0f;
        g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
        
        if (node.id == draggingId)
            g.setColour (juce::Colour::fromRGB (255, 0, 255));
    }
    
    g.setColour (juce::Colour::fromRGB (0, 255, 255));
    
    float freq = listener->getCurrPlayingFreq();
    const auto& point = coordsForEQNode (freq, juce::Decibels::gainToDecibels (curve.valueAtFrequency (freq).first.real()));
    float dotRadius = 3.0f;
    g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
    
    
}

juce::Point<float> CabinEQGraph::coordsForEQNode (float frequency, float amplitude)
{
    float width = getWidth();
    float height = getHeight();
    
    amplitude -= -4.5 * std::log2 (frequency / 1000.0f);
    
    float x = width * timeAtFrequency (frequency);
    float y = height * (1.0f - (amplitude + 24.0f) / 48.0f);
    
    return { x, y };
}

std::pair<float, float> CabinEQGraph::frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const
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

float CabinEQGraph::frequencyAtTime (float t) const
{
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(minFreqShowing);
    float logMaxFreq = std::log(maxFreqShowing);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    return freq;
}

float CabinEQGraph::timeAtFrequency (float freq) const
{
    float logMinFreq = std::log(minFreqShowing);
    float logMaxFreq = std::log(maxFreqShowing);
    float logFreq = std::log(freq);

    // Normalize the log frequency
    float t = (logFreq - logMinFreq) / (logMaxFreq - logMinFreq);
    return t;
}

bool CabinEQGraph::mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    float freqRatio = freq / eqNode.frequency;
    float amplDiff = std::abs (ampl - eqNode.amplitude);
    
    return freqRatio > 0.9 && freqRatio < 1.1 && amplDiff < 2.0;
}

float CabinEQGraph::mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    
    float freqRatio = freq / eqNode.frequency;
    
    return std::abs (1.0f - freqRatio);
}

std::optional<EQNode> CabinEQGraph::getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const
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

*/
