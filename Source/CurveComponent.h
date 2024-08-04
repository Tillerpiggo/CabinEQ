/*
  ==============================================================================

    CurveComponent.h
    Created: 14 Jun 2024 10:18:33am
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class CurveComponent : public juce::Component
{
public:
    CurveComponent (Curve& curve, Curve& curve2);
    CurveComponent& operator=(CurveComponent&& other) noexcept {
        return *this;
    }
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void drawTrueFrequencyResponse();
    
private:
    Curve& curve;
    Curve& curve2;
    std::vector<float> trueFreqResponse;
    
    int fftSize = 12;
    int numFreqResponsePoints = std::pow (2, fftSize);
};
*/
