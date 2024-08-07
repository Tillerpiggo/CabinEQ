/*
  ==============================================================================

    CabinEQPage.h
    Created: 27 Jul 2024 9:31:27pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNode.h"
#include "CabinEQGraph.h"

class CabinEQPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public CabinEQGraph::Listener
{
public:
    CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId);
    ~CabinEQPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    
    // CabinEQGraphListener methods
    int addNode (float freq, float ampl) override;
    void updateNode (int id, float freq, float ampl) override;
    void removeNode (int id) override;
    void startPlayingValueAt (float freq, float ampl) override;
    void playValueAt (float freq, float ampl) override;
    void testValueAt (float freq) override;
    void stopPlaying() override;
    void stopTesting() override;
    float getCurrPlayingFreq() override;
    float getCurrTestingFreq() override;
    
private:
    CabinEQGraph cabinEQGraph;
    
    StartupMVPAudioProcessor& processor;
    juce::String curveId;
    
    juce::Slider referenceSlider;
    
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    
    void updateEQNodes();
    
    std::vector<EQNode> eqNodes;
    juce::Colour backgroundColor = juce::Colour::fromRGB (0.4, 0.4, 0.4);
};
