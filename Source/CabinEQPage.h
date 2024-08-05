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
    
    void drawCurve (juce::Graphics& g, Curve& curve, int numPoints);
    juce::Colour getColorForFrequency(float frequency);
    void drawDots (juce::Graphics& g);
    juce::Point<float> coordsForEQNode (float frequency, float amplitude);
    
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    bool mouseEventIsNearEQNode (const juce::MouseEvent& event, EQNode eqNode) const;
    float mouseEventEQNodeDistance (const juce::MouseEvent& event, EQNode eqNode) const;
    float dbDistanceFromCurve (const float freq, const float ampl) const;
    
    std::optional<EQNode> getClosestEQNodeToMouseEvent (const juce::MouseEvent& event) const;
    
    void updateEQNodes();
    
    // Animation
    void updateSelectedDotSize();
    void rubberbandIfNotScrolling();
    
    std::vector<EQNode> eqNodes;
    
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float pendingMinFreqShowing = 20.0f;
    float pendingMaxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    float isScrollingTimer = 3;
    
    juce::OpenGLContext openGLContext;
    bool isTestingFreq = false;
    
    static constexpr float ANIM_STEP = 1.05f;
    static constexpr float DOT_SIZE_SELECTED = 5.5f;
    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
    static constexpr float DOT_PADDING = 3.0f;
    static constexpr float CURVE_THICKNESS = 2.5f;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    
    std::optional<float> targetSelectedDotSize;
    
    const juce::Colour I_LIKE_THE_ORANGE = juce::Colour::fromRGB(255, 180, 0);
    juce::Colour backgroundColor = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    static constexpr float DIST_TO_ADD_DB = 1.5f;
    std::optional<float> addingFreq;
    
    float lastDistanceFromDragStartX = 0;
};
