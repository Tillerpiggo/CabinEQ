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
    g.fillAll (juce::Colours::azure);
    // Draw a path with 1000 points using this->curve
    juce::Path path;
    path.startNewSubPath(0, 0);
    
    float width = getWidth();
    float height = getHeight();
    
    int N = 4000;
    
    for (int i = 0; i < N; i++)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>(N);
        float endX = width * normalizedTime;
        
        // Get value at time (val from 0 to 1)
        float val = curve.valueAtNormalizedTime (normalizedTime).real();
        
        // Transform to be in the height dimension
        float endY = height * (1.f - (val + 24.f) / 48.f);
        
        path.lineTo (endX, endY);
    }
    g.strokePath (path, juce::PathStrokeType (2.f));
}

void CurveComponent::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}
