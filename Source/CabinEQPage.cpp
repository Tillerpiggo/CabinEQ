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

void CabinEQPage::mouseDown (const juce::MouseEvent& event)
{
    float x = event.getMouseDownX();
    float y = event.getMouseDownY();
    auto [freq, ampl] = frequencyAndAmplitudeForCoords (x, y);
    
    juce::Point<float> clickedPoint (freq, ampl);
    float maxDist = 50.0f;
    
    // Get id of node within distance range
    for (const auto& eqNode : eqNodes)
    {
        juce::Point<float> eqNodePoint (eqNode.frequency, eqNode.amplitude);
        
        if (clickedPoint.getDistanceFrom (eqNodePoint) <= maxDist)
        {
            draggingId = eqNode.id;
            repaint();
            break;
        }
    }
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
        std::cout << "Frequency: " << node.frequency << ", Amplitude: " << node.amplitude << std::endl;
        g.fillEllipse (point.x - 3.0f, point.y - 3.0f, 6.0f, 6.0f); // Draw a small circle with radius 3
        
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

std::pair<float, float> CabinEQPage::frequencyAndAmplitudeForCoords (float x, float y) const
{
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
