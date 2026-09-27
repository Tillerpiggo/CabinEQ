/*
  ==============================================================================

    CabinPeqGraph.h
    Created: 10 Oct 2024 4:30:44pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <set>
#include "CabinEqAudioProcessor.h"
#include "BandEqCurve.h"
#include "Theme.h"

/// The EQ graph: the response curve over the live output spectrum, with a handle per band.
///
///  - Click the curve (or double-click anywhere) to add a band, and drag to shape it
///  - Drag a band to move it. Hold Cmd/Ctrl to move finely, Shift to keep to one axis
///  - Alt/Option-drag up and down, or scroll over a band, to change its width (Q)
///  - Drag across empty space to select several bands, and Shift-click to add or remove one
///  - Double-click a band to turn it off and on, and right-click for everything else
///  - Delete removes the selected bands, arrows nudge them, and Cmd/Ctrl+A selects all
class CabinPeqGraph  : public juce::Component,
                       private juce::Timer
{
public:
    explicit CabinPeqGraph (CabinEqAudioProcessor& processor);
    ~CabinPeqGraph() override;

    /// Re-reads the selected profile's bands. Call whenever the profiles change.
    void refresh();
    void setBypassed (bool isBypassed);

    /// The band the inspector shows: the last one clicked, or -1.
    int getFocusedBandId() const { return focusedId; }
    int getNumSelected() const { return (int) selectedIds.size(); }
    void focusBand (int bandId);
    std::function<void()> onSelectionChanged;

    void deleteSelectedBands();

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    enum class DragMode { none, bands, marquee };

    void timerCallback() override;
    void updateSpectrumTimer();

    // Drawing
    void drawGrid (juce::Graphics&);
    void drawSpectrum (juce::Graphics&);
    void drawCurves (juce::Graphics&);
    void drawHandles (juce::Graphics&);
    void drawReadout (juce::Graphics&);
    void rebuildCurvePaths();
    juce::Path curvePathForChannel (int channel) const;

    // Coordinates
    juce::Rectangle<float> getPlotArea() const;
    float xForFrequency (float frequency) const;
    float frequencyForX (float x) const;
    float yForDb (float db) const;
    float dbForY (float y) const;
    juce::Point<float> handlePosition (const Band& band) const;

    // Finding things under the mouse
    std::optional<Band> bandAt (juce::Point<float> position) const;
    bool isNearCurve (juce::Point<float> position) const;
    float curveDbNear (juce::Point<float> position) const; // of whichever curve (left or right) is nearer
    std::vector<Band> getSelectedBands() const;
    int indexOfBand (int bandId) const;

    // Editing
    CabinEqProfile profile() const { return processor.getSelectedProfile(); }
    void beginEdit (const juce::String& name, bool coalesce = false);
    int addBandAt (juce::Point<float> position, Band::Shape shape);
    void updateBands (const std::vector<Band>& bands);
    void changeWidth (const std::vector<Band>& bands, float factor);
    void nudge (float octaves, float db);
    void setSelection (std::set<int> ids, int newFocusedId);

    // Menus
    void showBandMenu (const Band& band);
    void showBackgroundMenu (juce::Point<float> position);

    // Display settings, kept in the state
    float getDisplayRange() const;
    void setDisplayRange (float db);
    bool getShowSpectrum() const;
    void setShowSpectrum (bool shouldShow);

    CabinEqAudioProcessor& processor;
    BandProfile bandProfile;
    BandEqCurve curve;
    juce::String profileName;

    std::set<int> selectedIds;
    int focusedId = -1;
    int hoverId = -1;
    bool hoverIsNearCurve = false;
    juce::Point<float> mousePosition;
    bool mouseIsOver = false;
    bool isBypassed = false;

    // Dragging
    DragMode dragMode = DragMode::none;
    bool isChangingWidth = false;
    juce::Point<float> lastDragPosition, dragDistance;
    std::vector<Band> bandsAtDragStart;
    juce::Rectangle<float> marquee;
    std::set<int> selectionBeforeMarquee;
    int bandAddedByLastClick = -1;
    juce::uint32 lastCoalescedEditTime = 0;
    juce::String lastCoalescedEditName;

    // Cached paths
    bool curvePathsNeedRebuilding = true;
    juce::Path mainCurve, leftCurve, rightCurve;

    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    static constexpr float handleRadius = 7.5f;
    static constexpr float axisHeight = 22.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinPeqGraph)
};
