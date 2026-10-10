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
#include <array>
#include "CabinEqAudioProcessor.h"
#include "BandEqCurve.h"
#include "Theme.h"

/// The EQ graph: the response curve over the live output spectrum, with a handle per band.
///
///  - Click the 0 dB line (or double-click anywhere) to add a band, and drag to shape it
///  - Drag a band to move it. Hold Cmd/Ctrl to move finely
///  - Shift-drag (or Alt-drag) up and down to change a band's width. Up is wider.
///    Shift can be pressed or let go mid-drag to switch between moving and widening
///  - Drag across empty space to select several bands, and Shift-click to add or remove one
///  - Double-click a band to turn it off and on, and right-click it to delete it
///  - Delete removes the selected bands, arrows nudge them, and Cmd/Ctrl+A selects all
///  - The -/+ in the top right, or dragging or scrolling on the dB axis, zooms the view (not the
///    bands' limits); double-click the axis to go back to +/-30 dB
///  - Scrolling (or pinching) anywhere else zooms in on the frequency under the mouse; scrolling
///    sideways, Shift-scrolling or dragging the frequency axis moves along them. Double-click the
///    frequency axis to see 20 Hz to 20 kHz again
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

    void deleteSelectedBands(); // or points, in curve mode

    /// Shows the calibration spots, which can be dragged along the frequency axis
    void setCalibrationSpotsVisible (bool shouldShow);

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void mouseMagnify (const juce::MouseEvent&, float scaleFactor) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    enum class DragMode { none, bands, points, rotate, marquee, zoom, spotsTogether, spotsResize, pan };

    // Curve mode: points the curve goes through, instead of bands
    bool isCurveMode() const { return editingCurve; } // which layer clicks and edits go to
    float responseDb (float frequency, int channel = -1) const; // of whichever the profile uses
    juce::Point<float> pointPosition (const CurvePoint& point) const;
    std::optional<CurvePoint> pointAt (juce::Point<float> position) const;
    std::vector<CurvePoint> getSelectedPoints() const;
    bool exists (int id) const; // a band or point with this id, whichever the mode
    int addPointAt (juce::Point<float> position, bool onCentreLine = true); // otherwise, exactly there
    void updatePoints (const std::vector<CurvePoint>& points);
    void curveMouseDown (const juce::MouseEvent&);
    void drawPoints (juce::Graphics&);
    int layer() const; // which of the curve's layers is being edited (a BandProfile::Layer): the shared curve unless split
    float pointDb (const CurvePoint& point) const; // where a point is drawn
    /// Curve mode's centre line, which new points are pulled out of: 0 dB, or for an ear's tweak, the shared curve
    bool isNearCentreLine (juce::Point<float> position) const;
    static juce::Colour earColour (int channel);
    int shownLayer = -1;
public:
    static inline const juce::Identifier idCurveLayer { "curveLayer" }; // on the state root, not undoable
    void setEditingLayer (int layer); // a BandProfile::Layer: both ears, or one ear's tweak, when split
    /// What the curve layer being edited sits on, and so what its points' gains are relative to: the bands
    /// if they're on, and for an ear's tweak, the shared curve as well. It's the dashed line.
    float layerBaseDb (float frequency) const;
private:
    std::vector<CurvePoint> pointsAtDragStart;

    // Experimental: Shift-drag a point to swing the points on one side of it about it, like a hinge.
    // Which side is the point's own: left of the graph's middle swings the left, right swings the right.
    CurvePoint rotatePivot;
    bool rotatesLeftSide = false;
    float rotateArmOctaves = 1.0f; // from the pivot to the edge of the view: a point out there follows the mouse
    float rotatedDb = 0.0f;
    bool wasCurveMode = false;
    bool editingCurve = false; // from the processor, for the profile as it is now

    // The -/+ dB range control in the top right
    struct ZoomControl { juce::Rectangle<float> bounds, minus, plus, label; };
    ZoomControl getZoomControl() const;
    void stepDisplayRange (int direction); // +1 zooms in, -1 zooms out
    bool isOnAxis (juce::Point<float> position) const;
    void drawZoomControl (juce::Graphics&);

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
    bool isHandleVisible (const Band& band) const;

    // Finding things under the mouse
    std::optional<Band> bandAt (juce::Point<float> position) const;
    bool isNearZeroLine (juce::Point<float> position) const; // where a click adds a band
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
    bool hoverIsNearZeroLine = false;
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
    float rangeAtDragStart = 30.0f;
    int shiftClickedId = -1;        // a shift-click on this band that hasn't turned into a drag yet
    bool hasBegunDragEdit = false;
    juce::uint32 lastCoalescedEditTime = 0;
    juce::String lastCoalescedEditName;

    // Cached paths
    bool curvePathsNeedRebuilding = true;
    juce::Path mainCurve, leftCurve, rightCurve, sharedCurve;
    juce::Path underCurve; // dashed: what the layer being edited sits on
    juce::Path pathFor (const std::function<float (float)>& dbAt) const;

    // Bands, and the widest view, go from 20 Hz to 20 kHz; the view can zoom in on part of that
    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    float viewLow = minFrequency, viewHigh = maxFrequency;
    void setFrequencyView (float low, float high);
    void zoomFrequencies (float factor, float aroundX); // factor < 1 zooms in
    void panFrequencies (float octaves);

    // Calibration spots shown on the graph
    bool showSpots = false;
    int draggingSpot = -1, hoverSpot = -1, shownSpot = -1;
    bool hoverAllSpots = false; // the mouse is on a spot's line, which moves them all
    juce::Rectangle<float> spotChip (int index) const;
    int spotChipAt (juce::Point<float> position) const;
    int spotLineAt (juce::Point<float> position) const;
    juce::Rectangle<float> spotGrip() const;          // between two spots, to move them both
    void beginSpotDrag (int spot, float x);           // -1 for the grip
    bool spotDragResizesTop = false;
    float spotDragAnchor = 1000.0f;
    std::array<float, CalibrationPlayer::maxSpots> spotFrequenciesAtDragStart {};
    void drawSpots (juce::Graphics&);
    static constexpr float handleRadius = 7.5f;
    static constexpr float axisHeight = 22.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinPeqGraph)
};
