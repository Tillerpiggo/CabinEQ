/*
  ==============================================================================

    CurveComponent.cpp
    Created: 14 Jun 2024 10:18:33am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CurveComponent.h"

void CurveComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(40, 40, 40));
    g.setColour (juce::Colour::fromRGB(0, 255, 128));

    juce::Path path;
    path.startNewSubPath(0, 0);
    
    float width = getWidth();
    float height = getHeight();
    
    int N = 4000;
    
    for (int i = 0; i < N; i++)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>(N);
        float endX = width * normalizedTime;
        
        float val = juce::Decibels::gainToDecibels (curve.valueAtNormalizedTime (normalizedTime).first.real());
        float endY = height * (1.f - (val + 24.f) / 48.f);
        
        path.lineTo (endX, endY);
    }
    g.strokePath (path, juce::PathStrokeType (2.f));
}

void CurveComponent::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}
