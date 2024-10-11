/*
  ==============================================================================

    CabinPeqGraph.h
    Created: 10 Oct 2024 4:30:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "BandProfile.h"

class CabinPeqGraph  : public juce::Component,
                       public juce::KeyListener,
                       public juce::Timer
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual int addBand (float freq, float ampl, float bandwidth, CabinPeqGraph* sender) = 0;
        virtual void updateBand (int id, float freq, float ampl, float bandwidth, CabinPeqGraph* sender) = 0;
        virtual void removeBand (int id, CabinPeqGraph* sender) = 0;
        virtual void startNoisePatternAt (int id, CabinPeqGraph* sender) = 0;
        virtual void updateNoisePatternAt (int id, CabinPeqGraph* sender) = 0;
        virtual void stopNoisePattern() = 0;
        virtual void setNoisePatternSolo (bool solo) = 0;
    };
    
    CabinPeqGraph();
    ~CabinPeqGraph() override;
    
    void setBands (std::vector<Band> bands);
    
    void paint (juce::Graphics& g) override;
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
    
private:
    std::vector<Band> bands;
    Listener* listener;
    
    // Drawing/animation
    void drawCurve (juce::Graphics& g);
    void drawDots (juce::Graphics& g);
    void drawDot (juce::Graphics& g, juce::Point<float> point, float radius, juce::Colour color);
    void updateHoveringStatus (const juce::MouseEvent& event); // updates what is being hovered over - whether it's a node or the center line
    
    // Colours
    juce::Colour getColourForFrequency (float frequency);
    
    // Coordinates
    juce::Point<float> coordsForFrequencyAndAmplitude (float freq, float ampl);
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    float mouseEventDistanceFromBand (const juce::MouseEvent& event, Band band) const; // distance from the node of the band
    std::optional<Band> getClosestBandToMouseEvent (const juce::MouseEvent& event) const; // which band's node is the closest to the mouse
    
    // Utils to handle calls to the listener if listener is nullptr
    int addBand (float freq, float ampl, float bandwidth);
    void updateBand (int id, float freq, float ampl, float bandwidth);
    void removeBand (int id);
    void startNoisePatternAt (int id);
    void updateNoisePatternAt (int id);
    void stopNoisePattern();
    void setNoisePatternSolo (bool solo);
    void soloNoisePatternIfAppropriate (const juce::MouseEvent& event);
    
    // Interaction variables
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    bool isHoveringOnCenterLine = false;
    bool isPlayingNoisePattern = false;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    
    // Dragging/zooming constants
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    float lastDistanceFromDragStartX = 0;
    
    // Constants
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr float DIST_TO_ADD_DB = 1.0f;
    static constexpr float HOVER_MIN_DIST = 0.5f;
    float MAX_DB = 36.0f;
    float MIN_DB = -36.0f;
    float DEFAULT_BANDWIDTH = 2.0f;
    
    // Visual constants
    static constexpr float DOT_SIZE_SELECTED = 5.5f;
    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
    static constexpr float DOT_PADDING = 3.0f;
    static constexpr float CURVE_THICKNESS = 2.5f;
    const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    // Visual flags
    bool isGrayscale = false;
};
