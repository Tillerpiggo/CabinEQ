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

class CabinEQPage   : public juce::Component,
                      public juce::Slider::Listener,
                      public juce::Timer

{
public:
    CabinEQPage (StartupMVPAudioProcessor& p, juce::String curveId);
    ~CabinEQPage() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void sliderValueChanged (juce::Slider *slider) override;
    void timerCallback() override;
    
private:
    StartupMVPAudioProcessor& processor;
    juce::String curveId;
    
    juce::Slider referenceSlider;
    
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    
    void drawCurve (juce::Graphics& g, const Curve& curve, int numPoints);
    juce::Colour getColorForFrequency(float frequency);
    void drawDots (juce::Graphics& g);
    juce::Point<float> coordsForEQNode (float frequency, float amplitude);
    
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    bool mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const;
    float mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const;
    
    std::optional<EQNode> getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const;
    
    void updateEQNodes();
    
    std::vector<EQNode> eqNodes;
    
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    
    juce::OpenGLContext openGLContext;
};
