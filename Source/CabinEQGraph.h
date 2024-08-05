/*
  ==============================================================================

    CabinEQGraph.h
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EQNode.h"

class CabinEQGraphListener
{
public:
    virtual ~CabinEQGraphListener() = default;

    virtual int addNode (float freq, float ampl);
    virtual void updateNode (int id, float freq, float ampl);
    virtual void removeNode (int id);
    virtual void playValueAt (float freq, float ampl); // the tone while dragging nodes
    virtual void testValueAt (float freq); // for probing
    virtual void stopPlaying(); // stops playing the calibration tones
    virtual void stopTesting(); // stops playing the testing tones (that are played when holding down ctrl/alt)
    
    virtual std::vector<EQNode> getEQNodes();
    virtual float getCurrPlayingFreq();
};

class CabinEQGraph   : public juce::Component,
                       public juce::Timer

{
public:
    CabinEQGraph (Curve& curve);
    ~CabinEQGraph() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void timerCallback() override;
    
    void addListener (CabinEQGraphListener* listener);
    void removeListener();
    
private:
    Curve& curve;
    std::vector<EQNode> eqNodes;
    CabinEQGraphListener* listener;
    
    // Drawing/animation
    void drawCurve (juce::Graphics& g, Curve& curve, int numPoints);
    juce::Colour getColorForFrequency(float frequency);
    void drawDots (juce::Graphics& g);
    juce::Point<float> coordsForEQNode (float frequency, float amplitude);
    void updateSelectedDotSize();
    
    // Utils
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    bool mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const;
    float mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const;
    float dbDistanceFromCurve (const float freq, const float ampl) const;
    std::optional<EQNode> getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const;
    void updateEQNodes();
    
    // Dragging/zooming
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    float lastDistanceFromDragStartX = 0;
    
    // Constants
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr float ANIM_STEP = 1.05f;
    static constexpr float DOT_SIZE_SELECTED = 5.5f;
    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
    static constexpr float DOT_PADDING = 3.0f;
    static constexpr float DIST_TO_ADD_DB = 1.0f;
    static constexpr float CURVE_THICKNESS = 2.5f;
    const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    // Micro-animation values
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    bool isTestingFreq = false;
    std::optional<float> addingFreq;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    std::optional<float> targetSelectedDotSize;
};
