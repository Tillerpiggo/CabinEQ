/*
  ==============================================================================

    CurveComponent.cpp
    Created: 14 Jun 2024 10:18:33am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CurveComponent.h"

CurveComponent::CurveComponent (const Curve& curve) : curve (curve)
{
    drawTrueFrequencyResponse();
}

void CurveComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(40, 40, 40));
    g.setColour (juce::Colour::fromRGB(0, 255, 128));

    juce::Path path;
    path.startNewSubPath(0, 0);
    
    float width = getWidth();
    float height = getHeight();
    
    int N = 4000;
    
    for (int i = 0; i < N; ++i)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>(N);
        float endX = width * normalizedTime;
        
        float val = juce::Decibels::gainToDecibels (curve.valueAtNormalizedTime (normalizedTime).first.real());
        float endY = height * (1.0f - (val + 24.0f) / 48.0f);
        
        path.lineTo (endX, endY);
    }
    g.strokePath (path, juce::PathStrokeType (2.0f));
    
    g.setColour (juce::Colours::transparentBlack);
    juce::Path trueFreqResponsePath;
    trueFreqResponsePath.startNewSubPath (0, 0);
    
    // Draw true frequency response
    for (int i = 0; i < numFreqResponsePoints; ++i)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>(numFreqResponsePoints);
        float endX = width * normalizedTime;
        
        float val = juce::Decibels::gainToDecibels (trueFreqResponse[i]);
        float endY = height * (1.0f - (val + 24.0f) / 48.0f);
        
        path.lineTo (endX, endY);
    }
    g.strokePath (trueFreqResponsePath, juce::PathStrokeType (1.0f));
}

void CurveComponent::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CurveComponent::drawTrueFrequencyResponse()
{
    auto [leftImpulseResponse, rightImpulseResponse] = curve.getStereoImpulse (10);
    
    // Perform 0-padded FFT
    juce::dsp::FFT fft (fftSize);
    float* zeroPaddedImpulse = new float[numFreqResponsePoints];
    
    for (int i = 0; i < numFreqResponsePoints * 2; ++i)
    {
        if (i < std::pow (2, 10) * 2) // Length of left impulse response, theoretically
        {
            zeroPaddedImpulse[i] = leftImpulseResponse[i];
        }
        else
        {
            zeroPaddedImpulse[i] = 0.0f;
        }
    }
    
    fft.performRealOnlyForwardTransform (zeroPaddedImpulse);
    
    // Save result to true freq response
    trueFreqResponse.clear();
    for (int i = 0; i < numFreqResponsePoints; i += 2)
    {
        trueFreqResponse.push_back (zeroPaddedImpulse[i]);
    }
    
    repaint();
}
