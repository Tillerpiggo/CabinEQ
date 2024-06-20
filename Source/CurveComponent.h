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
    CurveComponent (const Curve& curve) : curve (curve) {}
    CurveComponent& operator=(CurveComponent&& other) noexcept {
        return *this;
    }
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    const Curve& curve;
};
