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

class CabinPeqGraph  : public BuildableComponent,
                       public juce::Timer
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
    
    bool keyPressed(const juce::KeyPress& key) override;
    
    void timerCallback() override;
    
    void setListener (CabinPeqGraphListener* listener);
    void removeListener();
    
    void addDataSource (CabinPeqGraphDataSource* dataSource);
    void removeDataSource();
    
    void setGrayscale (bool grayscale);
    
private:
    void initializeLabels();
    void setFreqAmplLabelsBounds();
    void clearSelection(); // clears the current selection and all related variables
    
    CabinPeqGraphListener* listener;
    CabinPeqGraphDataSource* dataSource;
    
    BandProfile bandProfile;
    std::vector<Band> provisionalBands;
    std::vector<int> selectedBandIds;
    bool provisionalBandsVisible = false;
    
    juce::Label instructionLabel;
    std::string contactLong { "Contact us if anything breaks! julian@cabinaudio.com | tyler@cabinaudio.com" };
    std::string contactMid { "Contact us if anything breaks! julian@cabinaudio.com" };
    std::string contactShort { "Contact: julian@cabinaudio.com" };
    std::string addBandInstructions { "Click + drag on the center line to add a band" };
    std::string removeBandInstructions { "Right click to remove band" };
    std::string groupRemoveBandInstructions { "Right click to remove selected bands" };
    std::string shiftClickBandInstructions { "Shift click band to add it to group" };
    std::string shiftClickRemoveBandInstructions { "Shift click band to remove it from group" };
    std::string adjustBandwidthInstructions { "Shift + drag to change bandwidth" };
    std::string groupAdjustBandwidthInstructions { "Shift + drag to change bandwidth(s) of group" };
    std::string selectInstructions { "Drag to select multiple bands" };
    std::string groupDragInstructions { "Drag highlighted band to move or scale group" };
    std::string scrollInstructions { "Scroll vertically to zoom in/out" };
    std::string volumeInstructions { "Drag dot up/down to set volume for this profile" };
    
    juce::TextButton leftRightButton { "BOTH" };

    // Number labels
    std::array<juce::Label, 10> freqLabels;
    std::array<juce::Label, 11> amplLabels;
    
    // Drawing/animation
    void drawLines (juce::Graphics& g);
    void drawNoise (juce::Graphics& g);
    void drawBands (juce::Graphics& g);
    void drawCurve (juce::Graphics& g);
    void drawDots (juce::Graphics& g);
    void drawSelection (juce::Graphics& g);
    
    void drawBand (juce::Graphics& g, const Band& band, juce::Colour colour);
    
    std::vector<float> getLogLines();
    void drawDot (juce::Graphics& g, juce::Point<float> point, float radius, juce::Colour color, bool isSelected);
    void updateHoveringStatus (const juce::MouseEvent& event); // updates what is being hovered over - whether it's a node or the center line
    
    // Colours
    juce::Colour getColourForFrequency (float frequency);
    juce::ColourGradient getCurveGradient();
    
    // Coordinates
    std::pair<float, float> getEventCoords (const juce::MouseEvent& event) const;
    juce::Point<float> getLocalCoordsForBand (const Band band);
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
    int addBand (float freq, float ampl, float bandwidth, Band::Type type);
    void updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type);
    void updateBandFromDrag (const juce::MouseEvent& event);
    void removeBand (int id);
    void setVolume (float volume);
    
    // Interaction variables
    int draggingId = -1; // not currently dragging any point
    int hoveringId = -1; // not hovering over any point
    std::optional<float> addingFreq; // the frequency you are hovering over, if you're going to add a point. std::nullopt if you're not hovering in a place where you can add a node
    bool isHoveringOverDotControl; // if the mouse is hovering over the dot to control the volume of this profile
    bool isPlayingNoisePattern = false;
    float selectedDotSize = DOT_SIZE_DEFAULT;
    
    // Selection
    std::optional<juce::Point<float>> selectionStart;
    std::optional<juce::Point<float>> selectionEnd;
    std::optional<juce::Rectangle<float>> selectionRect;
    std::unordered_set<int> selectedIds;
    
    // Selection drag
    juce::Point<float> selectionStartPos; // the initial position you select for dragging
    std::unordered_map<int, Band> selectedIdToStartingValue; // the starting value for each selected band
    
    // Dragging/zooming constants
    float minFreqShowing = 16.0f;
    float maxFreqShowing = 22000.0f;
    float zoom = 5.0f;
    float lastDistanceFromDragStartX = 0;
    std::pair<float, float> dragOffsetWhileAdjustingBandwidth { 0.0f, 0.0f };
    std::pair<float, float> dragOffsetWhileAdjustingPosition { 0.0f, 0.0f };
    std::pair<float, float> lastDragPosition { 0.0f, 0.0f };
    std::pair<float, float> startDragPosition { 0.0f, 0.0f };
    float startDragBandwidth = 0.0f;
    std::vector<Band> startDraggingBands;
    
    // Constants
    static constexpr float MIN_FREQ = 16.0f;
    static constexpr float MAX_FREQ = 22000.0f;
    static constexpr float DIST_TO_ADD_DB = 1.0f;
    static constexpr float HOVER_MIN_DIST = 0.5f;
    float MAX_DB = 36.0f;
    float MIN_DB = -36.0f;
    float DEFAULT_BANDWIDTH = 1.0f;
    
    // Visual flags
    bool isGrayscale = false;
    
    // BandEqCurve
    BandEqCurve curve;
    Band::Type bandType = Band::Type::both;
    
    // Variables for faster painting
    
    // drawLines
    juce::PathStrokeType lineStrokeType { CURVE_THICKNESS / 2.0f};
    
    juce::Path centerPath;
    std::vector<juce::Path> horizontalLinePaths;
    std::vector<float> lineFreqs;
    
    int currStepId = 0; // for multi band steps
};
