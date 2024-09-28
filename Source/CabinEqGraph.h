/*
  ==============================================================================

    CabinEqGraph.h
    Created: 2 Aug 2024 2:51:41pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "CurvePt.h"

class CabinEqGraph   : public juce::Component,
                       public juce::KeyListener,
                       public juce::Timer
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual int addCurvePt (float freq, float ampl, CabinEqGraph* sender) = 0;
        virtual void updateCurvePt (int id, float freq, float ampl, CabinEqGraph* sender) = 0;
        virtual void removeCurvePt (int id, CabinEqGraph* sender) = 0;
        virtual void startPlayingValueAt (float freq, CabinEqGraph* sender) = 0;
        virtual void updatePlayingValueAt (float freq, CabinEqGraph* sender) = 0; // the tone while dragging nodes
        virtual void probeValueAt (float freq) = 0; // for probing
        virtual void stopPlaying() = 0; // stops playing the calibration tones
        virtual void stopProbing() = 0; // stops playing the probing tones (that are played when holding down ctrl/alt)
        virtual float getCurrPlayingFreq() = 0;
        virtual float getCurrProbingFreq() = 0;
        
        virtual void userStoppedDoingShit() = 0;
    };
    
    CabinEqGraph();
    ~CabinEqGraph() override;
    
    void setCurve (Curve& curve);
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    bool keyPressed (const juce::KeyPress &key, juce::Component *originatingComponent) override;
    bool keyStateChanged (bool isKeyDown, juce::Component *originatingComponent) override;
    
    void timerCallback() override;
    
    void addListener (Listener* listener);
    void removeListener();
    
    void setGrayscale (bool grayscale);
    void setBlinded (bool blinded);
    
private:
    std::optional<std::reference_wrapper<Curve>> curve;
    std::vector<CurvePt> curvePts;
    Listener* listener;
    
    // Drawing/animation
    void drawCurve (juce::Graphics& g, Curve& curve, int numPoints);
    void drawDots (juce::Graphics& g, Curve& curve);
    void drawDot (juce::Graphics& g, juce::Point<float> point, float radius, juce::Colour color);
    void updateSelectedDotSize();
    void updateHoveringAndAddingNode (const juce::MouseEvent& event);
    juce::ColourGradient getCurveGradient();
    juce::Colour getColorForFrequency(float frequency);
    juce::Point<float> coordsForCurvePt (float frequency, float val);
    
    // Utils
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    bool mouseEventIsNearCurvePt (const juce::MouseEvent& event, CurvePt curvePt) const;
    float mouseEventDistanceFromCurvePt (const juce::MouseEvent& event, CurvePt curvePt) const;
    float dbDistanceFromCurve (const float freq, const float ampl, Curve& curve) const;
    std::optional<CurvePt> getClosestCurvePtToMouseEvent (const juce::MouseEvent& event) const;
    void updateCurvePts();
    
    // Utils to handle listener being nullptr
    int addNode (float freq, float val);
    void updateNode (int id, float freq, float val);
    void removeNode (int id);
    void startPlayingValueAt (float freq);
    void updatePlayingValueAt (float freq); // the tone while dragging nodes
    void probeValueAt (float freq); // for probing
    void stopPlaying(); // stops playing the calibration tones
    void stopProbing(); // stops playing the testing tones (that are played when holding down ctrl/alt)
    float getCurrPlayingFreq();
    float getCurrProbingFreq();
    void userStoppedDoingShit();
    
    // Dragging/zooming
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    float lastDistanceFromDragStartX = 0;
    
    // Constants
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr float ANIM_STEP = 1.3f; // very fast animations for now
    static constexpr float DOT_SIZE_SELECTED = 5.5f;
    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
    static constexpr float DOT_PADDING = 3.0f;
    static constexpr float DIST_TO_ADD_DB = 1.0f;
    static constexpr float CURVE_THICKNESS = 2.5f;
    static constexpr float HOVER_MIN_DIST = 0.5f;
    const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    // Micro-animation values
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    bool isProbingFreq = false;
    bool isPlayingFreq = false;
    std::optional<float> addingFreq;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    std::optional<float> targetSelectedDotSize;
    
    bool grayscale = false;
    bool blinded = false;
    
    int cyclesSinceUserStoppedDoingShit = 0;
    
    // Scaling
    float maxDB = 36.0f;
    float minDB = -36.0f;
};
