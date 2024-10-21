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
#include "BandEqCurve.h"

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
        virtual void setVolume (float volume, CabinPeqGraph* sender) = 0;
    };
    
    // Provides information like the BandProfile, the currently playing freq, and other useful information
    class DataSource
    {
    public:
        virtual ~DataSource() = default;
        
        virtual BandProfile getBandProfile() = 0;
        virtual float getCurrPlayingFreq() = 0;
    };
    
    CabinPeqGraph();
    ~CabinPeqGraph() override;
    
    void setBandProfile (BandProfile bandProfile);
    
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
    
    void addDataSource (DataSource* dataSource);
    void removeDataSource();
    
    void setGrayscale (bool grayscale);
    
private:
    BandProfile bandProfile;
    Listener* listener;
    DataSource* dataSource;
    
    // Drawing/animation
    void drawLines (juce::Graphics& g);
    void drawBands (juce::Graphics& g);
    void drawCurve (juce::Graphics& g);
    void drawDots (juce::Graphics& g);
    
    std::vector<float> getLogLines();
    void drawDot (juce::Graphics& g, juce::Point<float> point, float radius, juce::Colour color, bool isSelected);
    void updateHoveringStatus (const juce::MouseEvent& event); // updates what is being hovered over - whether it's a node or the center line
    
    // Colours
    juce::Colour getColourForFrequency (float frequency);
    juce::ColourGradient getCurveGradient();
    
    // Coordinates
    std::pair<float, float> getEventCoords (const juce::MouseEvent& event) const;
    juce::Point<float> coordsForFrequencyAndAmplitude (float freq, float ampl);
    float xForFreq (float freq);
    float yForAmpl (float ampl);
    std::pair<float, float> frequencyAndAmplitudeForCoords (float x, float y) const;
    float frequencyAtTime (float t) const;
    float timeAtFrequency (float freq) const;
    std::pair<float, float> frequencyAndAmplitudeForMouseEvent (const juce::MouseEvent& event) const;
    float mouseEventDistanceFromBand (const juce::MouseEvent& event, Band band) const; // distance from the node of the band
    float mouseEventDistanceFromFrequencyAndAmplitude (const juce::MouseEvent& event, float freq, float ampl) const;
    std::optional<Band> getClosestBandToMouseEvent (const juce::MouseEvent& event) const; // which band's node is the closest to the mouse
    
    // Utils to handle calls to the listener if listener is nullptr
    int addBand (float freq, float ampl, float bandwidth);
    void updateBand (int id, float freq, float ampl, float bandwidth);
    void updateBandFromDrag (const juce::MouseEvent& event);
    void removeBand (int id);
    void startNoisePatternAt (int id);
    void updateNoisePatternAt (int id);
    void stopNoisePattern();
    void setNoisePatternSolo (bool solo);
    void soloNoisePatternIfAppropriate (const juce::MouseEvent& event);
    void setVolume (float volume);
    
    // Interaction variables
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    std::optional<float> addingFreq; // the frequency you are hovering over, if you're going to add a point. std::nullopt if you're not hovering in a place where you can add a node
    bool isHoveringOverDotControl; // if the mouse is hovering over the dot to control the volume of this profile
    bool isPlayingNoisePattern = false;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    
    // Dragging/zooming constants
    float minFreqShowing = 20.0f;
    float maxFreqShowing = 20000.0f;
    float zoom = 5.0f;
    float lastDistanceFromDragStartX = 0;
    std::pair<float, float> dragOffsetWhileAdjustingBandwidth { 0.0f, 0.0f };
    std::pair<float, float> dragOffsetWhileAdjustingPosition { 0.0f, 0.0f };
    std::pair<float, float> lastDragPosition { 0.0f, 0.0f };
    std::pair<float, float> startDragPosition { 0.0f, 0.0f };
    float startDragBandwidth = 0.0f;
    
    // Constants
    static constexpr float MIN_FREQ = 20.0f;
    static constexpr float MAX_FREQ = 20000.0f;
    static constexpr float DIST_TO_ADD_DB = 1.0f;
    static constexpr float HOVER_MIN_DIST = 0.5f;
    float MAX_DB = 36.0f;
    float MIN_DB = -36.0f;
    float DEFAULT_BANDWIDTH = 1.0f;
    
    // Visual constants
    static constexpr float NUM_POINTS = 1000; // num points used to render the curve
    static constexpr float DOT_SIZE_SELECTED = 5.5f;
    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
    static constexpr float DOT_PADDING = 3.0f;
    static constexpr float CURVE_THICKNESS = 2.5f;
    const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    // Visual flags
    bool isGrayscale = false;
    
    // BandEqCurve
    BandEqCurve curve;
};
