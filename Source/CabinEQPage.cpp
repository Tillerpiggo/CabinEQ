/*
  ==============================================================================

    CabinEQPage.cpp
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CabinEQPage.h"

CabinEQPage::CabinEQPage (StartupMVPAudioProcessor& p)
    : processor (p)
{
    updateEQNodes();
}

void CabinEQPage::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (34, 34, 34));
    drawCurve (g, processor.getCurve(), 4000);
    drawDots (g);
}

void CabinEQPage::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CabinEQPage::mouseMove (const juce::MouseEvent& event)
{
    for (const auto& eqNode : eqNodes)
    {
        if (mouseEventIsNearEQNode (event, eqNode))
        {
            hoveringId = eqNode.id;
            repaint();
            return;
        }
    }
    
    hoveringId = -1;
    repaint();
}

void CabinEQPage::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.startCalibratingEQNode (EQNode (-1, freq, processor.getCurve().valueAtFrequency (freq).first.real(), 0.0f));
        
        return;
    }
    
    // Get id of node within distance range
    for (const auto& eqNode : eqNodes)
    {
        if (mouseEventIsNearEQNode (event, eqNode))
        {
            draggingId = eqNode.id;
            //processor.startCalibratingEQNode (eqNode);
            
            if (! event.mods.isRightButtonDown())
            {
                processor.startCalibratingEQNode (eqNode);
            }
            else
            {
                processor.removeEQNode(draggingId);
            }
            
            repaint();
            return;
        }
    }
    
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    if (! event.mods.isRightButtonDown())
        processor.addEQNode (freq, ampl, 0.0f);
    
    repaint();
}

void CabinEQPage::mouseDrag (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        std::cout << "mouse drag!!" << std::endl;
        auto [freq, _] = frequencyAndAmplitudeForMouseEvent (event);
        processor.updateCalibratingEQNode (EQNode (-1, freq, processor.getCurve().valueAtFrequency (freq).first.real(), 0.0f));
        
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
    
    processor.updateEQNode (draggingId, freq, ampl, node.pan);
    processor.updateCalibratingEQNode (node);
    repaint();
}

void CabinEQPage::mouseUp (const juce::MouseEvent& event)
{
    auto [freq, ampl] = frequencyAndAmplitudeForMouseEvent (event);
    EQNode node (0, 0, 0, 0);
    // Get the eq node in question
    for (const auto& eqNode : eqNodes)
        if (eqNode.id == draggingId)
            node = eqNode;
    
    node.frequency = freq;
    node.amplitude = ampl;
    
    processor.updateEQNode (draggingId, freq, ampl, node.pan);
    processor.endCalibratingEQNode();
    draggingId = -1;
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
        
        float dbDifference = -4.5f * std::log2((node.frequency) / 1000.0f);
        const auto& point = coordsForEQNode (node.frequency, node.amplitude - dbDifference);
        
        float dotRadius = 4.0f;
        if (node.id == hoveringId)
            dotRadius = 6.0f;
        g.fillEllipse (point.x - dotRadius, point.y - dotRadius, dotRadius * 2, dotRadius * 2);
        
        if (node.id == draggingId)
            g.setColour (juce::Colour::fromRGB (255, 0, 255));
    }
}

juce::Point<float> CabinEQPage::coordsForEQNode (float frequency, float amplitude)
{
    float width = getWidth();
    float height = getHeight();
    
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
    float dbDifference = -4.5f * std::log2((freq) / 1000.0f);
    ampl += dbDifference;
    
    return { freq, ampl };
}

float CabinEQPage::frequencyAtTime (float t) const
{
    // Scale logarithmically (should this be here?)
    float logMinFreq = std::log(MIN_FREQ);
    float logMaxFreq = std::log(MAX_FREQ);
    float freq = std::exp(logMinFreq + t * (logMaxFreq - logMinFreq));
    
    return freq;
}

float CabinEQPage::timeAtFrequency (float freq) const
{
    float logMinFreq = std::log(MIN_FREQ);
    float logMaxFreq = std::log(MAX_FREQ);
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
