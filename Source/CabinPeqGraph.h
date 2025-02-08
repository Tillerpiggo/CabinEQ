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
#include "UIConstants.h"
#include "Listeners.h"
#include "BuildableComponent.h"
#include "DimensionalSlider.h"

class CabinPeqGraph  : public BuildableComponent,
                       public juce::Timer,
                       public DimensionalSlider::Listener
{
public:
    
    
    CabinPeqGraph();
    ~CabinPeqGraph() override;
    
    void setBandProfile (BandProfile bandProfile);
    void setProvisionalBands (std::vector<Band> provisionalBands);
    void setProvisionalBandsVisible (bool provisionalBandsVisible);
    void updateBands();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void positionChanged (juce::Point<float> pos) override;
    
    void timerCallback() override;
    
    void setListener (CabinPeqGraphListener* listener);
    void removeListener();
    
    void addDataSource (CabinPeqGraphDataSource* dataSource);
    void removeDataSource();
    
    void setGrayscale (bool grayscale);
    
private:
    void updateContactLabelText(); // updates the contact label text depending on the size of the window
    
    CabinPeqGraphListener* listener;
    CabinPeqGraphDataSource* dataSource;
    
    BandProfile bandProfile;
    std::vector<Band> provisionalBands;
    std::vector<int> selectedBandIds;
    bool provisionalBandsVisible = false;
    
    juce::Label instructionLabel;
//    juce::Label contactLabel;
    std::string contactLong { "Contact us if anything breaks! julian@cabinaudio.com | tyler@cabinaudio.com" };
    std::string contactMid { "Contact us if anything breaks! julian@cabinaudio.com" };
    std::string contactShort { "Contact: julian@cabinaudio.com" };
    std::string addBandInstructions { "Click + drag on the center line to add a band" };
    std::string removeBandInstructions { "Right click to remove band" };
    std::string adjustBandwidthInstructions { "Shift + drag to adjust bandwidth" };
    std::string scrollInstructions { "Scroll vertically to zoom in/out" };
    std::string volumeInstructions { "Drag dot up/down to set volume for this profile" };
    
    DimensionalSlider dimensionalSlider;
    juce::TextButton leftRightButton { "BOTH" };
    
    // Drawing/animation
    void drawLines (juce::Graphics& g);
    void drawNoise (juce::Graphics& g);
    void drawBands (juce::Graphics& g);
    void drawCurve (juce::Graphics& g);
    void drawDots (juce::Graphics& g);
    
    void drawBand (juce::Graphics& g, const Band& band, juce::Colour colour);
    
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
    
    // Utils for dimensional slider nonsense
    std::pair<float, float> dimensionalSliderPosToFreqs (juce::Point<float> pos);
    juce::Point<float> freqsToDimensionalSliderPos (std::pair<float, float> freqs);
    float dimensionalMinFreq = 20.0f;
    float dimensionalMaxFreq = 20000.0f;
    
    // Utils to handle calls to the listener if listener is nullptr
    int addBand (float freq, float ampl, float bandwidth, Band::Type type);
    void updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type);
    void updateBandFromDrag (const juce::MouseEvent& event);
    void removeBand (int id);
    void setVolume (float volume);
    
    // Interaction variables
    std::optional<float> selectionStartFreq;
    std::optional<float> selectionEndFreq;
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    std::optional<float> addingFreq; // the frequency you are hovering over, if you're going to add a point. std::nullopt if you're not hovering in a place where you can add a node
    bool isHoveringOverDotControl; // if the mouse is hovering over the dot to control the volume of this profile
    bool isPlayingNoisePattern = false;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    
    // Selection
    
    
    
    
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
    std::vector<Band> startDraggingBands;
    
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
//    static constexpr float DOT_SIZE_SELECTED = 5.5f;
//    static constexpr float DOT_SIZE_DRAGGING = 8.0f;
//    static constexpr float DOT_SIZE_DEFAULT = 3.5f;
//    static constexpr float DOT_PADDING = 3.0f;
//    static constexpr float CURVE_THICKNESS = 2.5f;
//    const juce::Colour BACKGROUND_COLOR = juce::Colour::fromRGB (0.1, 0.1, 0.2);
    
    // Visual flags
    bool isGrayscale = false;
    
    // BandEqCurve
    BandEqCurve curve;
    Band::Type bandType = Band::Type::both;
    
    // Variables for faster painting
    
    // drawLines
    juce::Colour centerLineColour = juce::Colours::darkgrey.withMultipliedLightness (0.5f);
    juce::Colour lineColour = juce::Colours::darkgrey.withMultipliedLightness (0.5f);//.withAlpha (0.3f);
    juce::PathStrokeType lineStrokeType { CURVE_THICKNESS / 2.0f};
    
    juce::Path centerPath;
    std::vector<juce::Path> horizontalLinePaths;
    std::vector<float> lineFreqs;
    
    int currStepId = 0; // for multi band steps
};
