/*
  ==============================================================================

    ElevationCalibrationView.h
    Created: 23 Feb 2025 11:26:04am
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"
#include "Listeners.h"

// This class provides a UI for elevation calibration, displaying multiple rows and allowing you to select one.
class ElevationCalibrationView  : public BuildableComponent,
                                  public juce::Timer
{
public:
    ElevationCalibrationView();
    ~ElevationCalibrationView() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;

    void setListener (ElevationCalibrationListener* listener);
    void setDataSource (ElevationCalibrationDataSource* dataSource);

    void updateElevationCalibration();

    void timerCallback() override;
    
private:
    void updatePlusMinusButtons();

    void drawGridLines (juce::Graphics& g);
    void drawRows (juce::Graphics& g);
    void drawRow (int rowIdx, juce::Graphics& g);

    juce::Rectangle<float> getRowRect (int rowIdx);
    std::optional<int> rowIdxForMouseEvent (const juce::MouseEvent& event);

    ElevationCalibrationListener* listener = nullptr;
    ElevationCalibrationDataSource* dataSource = nullptr;

    juce::TextButton plusButton;
    juce::TextButton minusButton;

    bool isPlaying = false;

    // Gestures
    int numRows = 3;
    int selectedRow = 1; // 0-indexed, 0 = lower frequency
    

    // Visual constants
    juce::Colour GRIDLINE_COLOUR = juce::Colours::teal;
    juce::Colour ROW_COLOUR = juce::Colours::teal;
    juce::Colour HOVER_COLOUR = juce::Colours::black.withAlpha (0.2f);
};


