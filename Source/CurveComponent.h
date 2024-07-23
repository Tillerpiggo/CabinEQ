/*
  ==============================================================================

    CurveComponent.h
    Created: 14 Jun 2024 10:18:33am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class CurveComponent : public juce::Component
{
public:
    CurveComponent (const Curve& curve);
    CurveComponent& operator=(CurveComponent&& other) noexcept {
        return *this;
    }
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void drawTrueFrequencyResponse();
    
private:
    const Curve& curve;
    std::vector<float> trueFreqResponse;
    
    int fftSize = 18;
    int numFreqResponsePoints = std::pow (2, fftSize);
};
