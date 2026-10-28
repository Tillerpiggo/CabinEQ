/*
  ==============================================================================

    CalibrationPanel.h

    A grid of positions that play pink noise bursts in reading order, to check the
    EQ by ear. Columns go from your left ear to your right, rows from high (top)
    to low: each row up cuts off more of the lows. Click positions to play just
    those (Shift- or Cmd-click, or drag, to pick several), and move them with the
    arrow keys (Shift+arrow to add the next one). It has its own volume, and a
    depth that plays each position several times, getting louder.

    "On graph", the default, plays 2 to 4 spots instead, each from its base frequency
    (a sharp low cut) up to 20 kHz. You shape them on the EQ graph. They share a pan
    range, and pan steps (which stack with depth) sweep them across it. They show on the EQ graph, where you can drag
    them, so you can line them up with the bands exactly.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"
#include "ValueField.h"
#include "GuidedCalibrationView.h"

class CalibrationPanel : public juce::Component,
                         private juce::Timer
{
public:
    explicit CalibrationPanel (CabinEqAudioProcessor& processor);
    ~CalibrationPanel() override;

    std::function<void()> onCloseClicked;
    std::function<void()> onModeChanged; // the graph shows the spots in "On graph" mode
    void stop();
    bool isShowingSpots() const;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override; // arrows move the selection, Shift+arrows grow it
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    static constexpr int preferredHeight = 390;
    juce::Rectangle<int> getFieldArea() const { return gridArea; } // where the grid, or Drag mode's field, is

private:
    void timerCallback() override;
    void applySettings();
    void setMode (CalibrationPlayer::Mode mode);
    void refreshSpots();
    void updateButtons();
    int positionAt (juce::Point<float> point) const;
    juce::Point<float> centreOf (int position) const;
    int rows() const { return (int) rowsSlider.getValue(); }
    int columns() const { return (int) columnsSlider.getValue(); }

    CabinEqAudioProcessor& processor;
    CalibrationPlayer& player;

    juce::TextButton playButton { "Play" }, allButton { "Play all" }, closeButton { "Hide" };
    juce::Slider repeatsSlider; // how many times a row plays at each depth level
    juce::Label repeatsLabel;
    std::array<juce::TextButton, 3> depthUnitButtons; // what the depth works on: each dot, each row, or the whole cycle
    juce::Label depthUnitLabel;
    void setDepthUnit (CalibrationPlayer::DepthUnit unit);
    void updateDepthControls(); // which of depth, repeats and the floor mean anything in this mode
    juce::Slider rowsSlider, columnsSlider, volumeSlider, depthSlider, speedSlider, attackSlider, releaseSlider, floorSlider;
    juce::Label volumeLabel, depthLabel, speedLabel, attackLabel, releaseLabel, floorLabel;
    juce::Slider slopeSlider, topLineSlider; // the noise: how fast a burst falls away, and how fast their bottom edges do
    juce::Label slopeLabel, topLineLabel;
    juce::Slider lowPassSlider; // how wide each burst's band is, in octaves or mels
    juce::Label lowPassLabel;
    juce::Slider bandPassSlider; // a band pass on top of everything: its low cut and its low-pass, two thumbs
    juce::Label bandPassLabel, bandPassText;
    void updateBandPassText();
    std::array<juce::TextButton, 6> topEdgeButtons; // where the bursts' tops are: fixed, a band in octaves or mels, or the grid's segments
    juce::Label topEdgeLabel;
    void setTopEdge (CalibrationPlayer::TopEdge edge);
    juce::Rectangle<int> rowsCaption, columnsCaption;

    juce::TextButton gridModeButton { "Grid" }, spotsModeButton { "On graph" }, squareModeButton { "Drag" }, guidedModeButton { "Guided" }, rangeModeButton { "Range" };

    // Range mode: a range you choose, as a bottom, a middle and a top, played top to bottom
    juce::Slider rangeSlider;  // the range, two thumbs
    juce::Slider middleSlider; // the middle's share of it
    juce::Label rangeLabel, middleLabel;
    juce::Rectangle<int> rangeBars; // the three bands, drawn with the one playing lit
    bool isRangeMode() const { return mode() == CalibrationPlayer::Mode::range; }
    void applyRange();
    void paintRangeMode (juce::Graphics&);

    // How the low cuts are spaced up the grid, Drag mode's field and the guided arrangements: in octaves, mels or Hz
    std::array<juce::TextButton, 3> spacingButtons;
    juce::Label spacingLabel;
    void setHeightSpacing (CalibrationPlayer::HeightSpacing spacing);
    juce::TextButton pinButton { "Pin" }; // pins the bottom and top heights to frequencies of your own
    void showHeightPins();
    juce::TextButton panButton { "Pan" }; // how positions across are spaced, and the pan law
    void showPanOptions();
    void setHeightPins (bool shouldPin, float low, float high);

    // Guided mode (experimental): a search that gives you one change at a time to try. It plays its own arrangements.
    GuidedCalibrationView guidedView;
    bool isGuidedMode() const { return mode() == CalibrationPlayer::Mode::sequence; }

    // Drag mode: a square of dots you put anywhere on the field (the grid's area). Across is pan; up is the low cut.
    std::array<juce::TextButton, 2> squareSizeButtons; // 2 x 2 and 3 x 3
    enum class SquareDrag { none, move, resize };
    SquareDrag squareDrag = SquareDrag::none;
    juce::Point<float> squareGrab; // from the square's centre to where it was grabbed, on the field
    CalibrationPlayer::Mode mode() const;
    bool isSquareMode() const { return mode() == CalibrationPlayer::Mode::square; }
    juce::Point<float> toPanel (juce::Point<float> onField) const;
    juce::Point<float> toField (juce::Point<float> onPanel) const;
    juce::Rectangle<float> squareOutline() const; // around its dots
    juce::Rectangle<float> squareHandle() const;  // the corner that resizes it
    void setSquare (CalibrationPlayer::Square square);
    void paintSquareMode (juce::Graphics&);
    void startPlaying();
    std::array<juce::TextButton, CalibrationPlayer::maxSpots - 1> spotCountButtons; // 2, 3 and 4 spots
    juce::Slider spotsPan;       // the pan range they all share, with two thumbs
    juce::Slider panStepsSlider; // how many positions across it
    juce::Label panStepsLabel;
    juce::Rectangle<int> spotsArea, spotDots;

    juce::Rectangle<int> gridArea;
    int hoverPosition = -1, shownPosition = -1;
    int cursor = -1; // where the arrow keys move from: the last position clicked or moved to
    bool dragAdds = true; // whether dragging across positions selects them or unselects them

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CalibrationPanel)
};
