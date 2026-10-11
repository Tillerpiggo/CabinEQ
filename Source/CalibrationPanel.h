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
    bool keyPressed (const juce::KeyPress&) override; // arrows move the selection, Shift+arrows grow it
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    static constexpr int preferredHeight = 260;

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
    juce::Slider slopeSlider, topLineSlider; // the noise: how fast a burst falls away, and how fast their bottom edges do
    juce::Label slopeLabel, topLineLabel;
    juce::Slider rowsSlider, columnsSlider, volumeSlider, depthSlider, speedSlider, attackSlider, releaseSlider, floorSlider;
    juce::Label volumeLabel, depthLabel, speedLabel, attackLabel, releaseLabel, floorLabel;
    juce::Rectangle<int> rowsCaption, columnsCaption;

    juce::TextButton gridModeButton { "Grid" }, spotsModeButton { "On graph" };
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
