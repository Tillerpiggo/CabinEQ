/*
  ==============================================================================

    CalibrationPanel.h

    A grid of positions that play pink noise bursts in reading order, to check the
    EQ by ear. Columns go from your left ear to your right, rows from high (top)
    to low. Click a position to repeat just that one. It has its own volume.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CabinEqAudioProcessor.h"

class CalibrationPanel : public juce::Component,
                         private juce::Timer
{
public:
    explicit CalibrationPanel (CabinEqAudioProcessor& processor);
    ~CalibrationPanel() override;

    std::function<void()> onCloseClicked;
    void stop();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    static constexpr int preferredHeight = 230;

private:
    void timerCallback() override;
    void applySettings();
    void updateButtons();
    int positionAt (juce::Point<float> point) const;
    juce::Point<float> centreOf (int position) const;
    int rows() const { return (int) rowsSlider.getValue(); }
    int columns() const { return (int) columnsSlider.getValue(); }

    CabinEqAudioProcessor& processor;
    CalibrationPlayer& player;

    juce::TextButton playButton { "Play" }, allButton { "Play all" }, closeButton { "Hide" };
    juce::Slider rowsSlider, columnsSlider, volumeSlider;
    juce::Label rowsLabel, columnsLabel, volumeLabel;

    juce::Rectangle<int> gridArea;
    int hoverPosition = -1, shownPosition = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CalibrationPanel)
};
