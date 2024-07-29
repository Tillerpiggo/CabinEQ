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

class CabinEQPage   : public juce::Component
{
public:
    CabinEQPage (StartupMVPAudioProcessor& p);
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    
private:
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    
    void drawCurve (juce::Graphics& g, const Curve& curve, int numPoints);
    void drawDots (juce::Graphics& g);
    juce::Point<float> coordsForEQNode (float frequency, float amplitude);
    
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    bool mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const;
    EQNode eqNodeForMouseEvent (const juce::MouseEvent& event) const; // returns an EQNode corresponding to the point on the curve at the frequency of the mouse event (x-value)
    
    void updateEQNodes();
    
    StartupMVPAudioProcessor& processor;
    std::vector<EQNode> eqNodes;
    
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
};
