/*
  ==============================================================================

    MagicKnob.h
    Created: 30 Dec 2024 6:47:55pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Layout.h"
#include "BuildableComponent.h"
#include "BandProfile.h"
#include "MagicDecoder.h"

//#include <tensorflow/cc/client/client_session.h>
//#include <tensorflow/cc/ops/standard_ops.h>
//#include <tensorflow/core/framework/tensor.h>
//#include <iostream>

// This provides a UI to adjust knobs, and to have those knobs recommend changes to the currently selected bands
class MagicKnob  : public BuildableComponent
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void setBands (std::vector<Band> bands) = 0;
    };
    
    MagicKnob();
    ~MagicKnob() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    
private:
    void updateListener(); // updates the listener with the new bands that should be selected, if the listener is available to listen
    
    Listener* listener = nullptr;
    std::unique_ptr<MagicDecoder> magicDecoder;
    
    juce::Slider slider1;
    juce::Slider slider2;
    juce::Slider slider3;
    juce::Slider slider4;
    juce::Slider slider5;
    juce::Slider slider6;
    juce::Slider slider7;
    juce::Slider slider8;
    
    juce::Label label1;
    juce::Label label2;
    juce::Label label3;
    juce::Label label4;
    juce::Label label5;
    juce::Label label6;
    juce::Label label7;
    juce::Label label8;
};
