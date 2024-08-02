/*
  ==============================================================================

    CurveComponent.cpp
    Created: 14 Jun 2024 10:18:33am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CurveComponent.h"

CurveComponent::CurveComponent (const Curve& curve, const Curve& curve2) : curve (curve), curve2 (curve2)
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
    
    float windowFactor = 0.03;
    
    int N = 4000;
    
    for (int i = 0; i < N; ++i)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>(N);
        float endX = width * normalizedTime;
        
        float val1 = juce::Decibels::gainToDecibels (curve.valueAtNormalizedTime (normalizedTime).first.real());
        float val2 = juce::Decibels::gainToDecibels (curve2.valueAtNormalizedTime (normalizedTime).first.real());
        
        float val = val1 - val2;
        float endY = height * (1.0f - (val + 24.0f) / 48.0f);
        
        path.lineTo (endX, endY);
    }
    g.strokePath (path, juce::PathStrokeType (1.0f));
    
//    for (int i = 0; i < N; ++i)
//    {
//        float normalizedTime = static_cast<float>(i) / static_cast<float>(N);
//        float endX = width * normalizedTime;
//        
//        float val = juce::Decibels::gainToDecibels (curve.valueAtTime (normalizedTime * windowFactor).first.real());
//        float endY = height * (1.0f - (val + 24.0f) / 48.0f);
//        
//        path.lineTo (endX, endY);
//    }
//    g.strokePath (path, juce::PathStrokeType (1.0f));
    
    /*
    g.setColour (juce::Colours::transparentBlack);
    juce::Path trueFreqResponsePath;
    trueFreqResponsePath.startNewSubPath (0, 0);
    
    g.setColour (juce::Colour::fromRGB(255, 128, 0));
    
    // Draw true frequency response
    for (int i = 0; i < (trueFreqResponse.size() / 2) * windowFactor; ++i)
    {
        float normalizedTime = static_cast<float>(i) / static_cast<float>((trueFreqResponse.size() / 2) * windowFactor);
        float endX = width * normalizedTime;
        
        float val = juce::Decibels::gainToDecibels (trueFreqResponse[i]);
        float endY = height * (1.0f - (val + 15.1f) / 48.0f);
        
        trueFreqResponsePath.lineTo (endX, endY);
    }
    g.strokePath (trueFreqResponsePath, juce::PathStrokeType (1.0f));
     */
}

void CurveComponent::resized()
{
    setBounds (0, 0, getWidth(), getHeight());
}

void CurveComponent::drawTrueFrequencyResponse()
{
    int impulseLen = 10;
    int impulseNumPoints = std::pow (2, impulseLen);
    auto [leftImpulseResponse, rightImpulseResponse] = curve.getStereoImpulse (impulseLen);
    
    // Perform 0-padded FFT
    juce::dsp::FFT fft (fftSize);
    float* zeroPaddedImpulse = new float[fft.getSize() * 2];
    
    for (int i = 0; i < fft.getSize() * 2; ++i)
    {
        if (i < impulseNumPoints * 2) // Length of left impulse response, theoretically
        {
            zeroPaddedImpulse[i] = leftImpulseResponse[i];
        }
        else
        {
            zeroPaddedImpulse[i] = 0.0f;
        }
    }
    
//    std::cout << "Zero padded impulse: " << std::endl;
//    for (int i = 0; i < numFreqResponsePoints * 2; ++i) std::cout << zeroPaddedImpulse[i] << " ";
//    std::cout << std::endl;
 
    fft.performFrequencyOnlyForwardTransform (zeroPaddedImpulse);
    
    // Save result to true freq response
    trueFreqResponse.clear();
    for (int i = 0; i < numFreqResponsePoints; i += 2)
    {
        trueFreqResponse.push_back (zeroPaddedImpulse[i]);
    }
    
    repaint();
    
    delete[] leftImpulseResponse;
    delete[] rightImpulseResponse;
    delete[] zeroPaddedImpulse;
}
