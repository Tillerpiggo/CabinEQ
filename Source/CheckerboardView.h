/*
  ==============================================================================

    CheckerboardView.h
    Created: 10 Feb 2025 2:58:24pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Listeners.h"
#include "Checkerboard.h"

// This view displays a dynamic checkerboard that represents a given checkerboard. It also includes an interface for changing the displayed checkerboard interactively (not yet)
class CheckerboardView  : public juce::Component
{
public:
    CheckerboardView();
    ~CheckerboardView();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setDataSource (CheckerboardViewDataSource* dataSource);
    void updateIsPlaying(); // signals isPlaying is changed, triggers update of visuals
    void updateCheckerboard(); // signals the checkerboard changes, also triggers update of visuals
private:
    void drawGridLines (juce::Graphics& g);
    void drawSquares (juce::Graphics& g);
    
    void drawSquare (int freqIdx, int panIdx, juce::Colour squareColour, juce::Graphics& g); // uses Checkerboard info to draw a square at the given location
    juce::Rectangle<float> getRectForFreqIdxAndPanIdx (int freqIdx, int panIdx); // based on checkerboard, gets the associated coords for this freq/pan idx. Returns a rectangle with 0 size if freq idx or pan idx are out of bounds
    
    CheckerboardViewDataSource* dataSource = nullptr;
    
    bool isPlaying = false;
    Checkerboard checkerboard;
    
    // Visual constants
    juce::Colour BORDER_COLOUR = juce::Colours::teal;
    juce::Colour GRIDLINE_COLOUR = juce::Colours::teal;
    juce::Colour SQUARE_ON_COLOUR = juce::Colours::teal;
    juce::Colour SQUARE_OFF_COLOUR = juce::Colours::teal.withAlpha (0.3f);
    juce::Colour SQUARE_EMPTY_COLOUR = juce::Colours::black;
    
};
