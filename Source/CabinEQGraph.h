/*
  ==============================================================================

    CabinEQGraph.h
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

/*
#pragma once

#include <JuceHeader.h>
#include "Curve.h"

class CabinEQGraph   : public juce::Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void nodeAdded (float freq, float ampl);
        virtual void nodeMoved (int id, float freq, float ampl);
        virtual void nodeDeleted (int id);
        virtual void playValueAt (float freq, float ampl); // the tone while dragging nodes
        virtual void testValueAt (float freq); // for probing
    };
    
    CabinEQGraph (const Curve& curve);
    ~CabinEQGraph() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void addListener (SequencerListener* newListener);
    void removeListener();
    
private:
    Listener* listener;
    
    void updateEQNodes();
    void drawCurve (juce::Graphics& g, const Curve& curve, int numPoints);
    void drawDots (juce::Graphics& g);
    
    juce::Point<float> coordsForEQNode (float frequency, float amplitude);
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    
    bool mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const;
    float mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    std::optional<EQNode> getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const;
    
    const Curve& curve;
    std::vector<EQNode> eqNodes;
    
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
};
*/
