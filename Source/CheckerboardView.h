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
class CheckerboardView  : public juce::Component,
                          public juce::Timer
{
public:
    
    
    CheckerboardView();
    ~CheckerboardView();
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseExit (const juce::MouseEvent &event) override;
    
    void setListener (CheckerboardViewListener* listener);
    void setDataSource (CheckerboardViewDataSource* dataSource);
    void updateIsPlaying(); // signals isPlaying is changed, triggers update of visuals
    void updateCheckerboard(); // signals the checkerboard changes, also triggers update of visuals
    
    void timerCallback() override;
private:
    void drawGridLines (juce::Graphics& g);
    void drawSquares (juce::Graphics& g);
    void drawSoloBorder (juce::Graphics& g);
    void selectBetween (std::pair<int, int> point1, std::pair<int, int> point2); // selects/solos the coordinates between the two given points
    
    void drawSquare (int freqIdx, int panIdx, juce::Colour squareColour, juce::Graphics& g); // uses Checkerboard info to draw a square at the given location
    juce::Rectangle<float> getRectForFreqIdxAndPanIdx (int freqIdx, int panIdx); // based on checkerboard, gets the associated coords for this freq/pan idx. Returns a rectangle with 0 size if freq idx or pan idx are out of bounds
    std::optional<std::pair<int, int>> coordsForMouseEvent (const juce::MouseEvent &event); // gets the [freqIdx, panIdx] for a given mouse event
    
    CheckerboardViewListener* listener = nullptr;
    CheckerboardViewDataSource* dataSource = nullptr;
    
    bool isPlaying = false;
    Checkerboard checkerboard;
    
    // Visual constants
    juce::Colour BORDER_COLOUR = juce::Colours::teal;
    juce::Colour GRIDLINE_COLOUR = juce::Colours::teal;
    juce::Colour SQUARE_ON_COLOUR = juce::Colours::teal;
    juce::Colour SQUARE_OFF_COLOUR = juce::Colours::teal.withAlpha (0.3f);
    juce::Colour SQUARE_EMPTY_COLOUR = juce::Colours::teal.withAlpha (0.1f);
    juce::Colour SOLO_BORDER_COLOUR = juce::Colours::skyblue;
//    juce::Colour SOLO_SQUARE_ON_COLOUR = juce::Colours::purple;
//    juce::Colour SOLO_SQUARE_OFF_COLOUR = juce::Colours::purple.withAlpha (0.3f);
//    juce::Colour SOLO_SQUARE_EMPTY_COLOUR = juce::Colours::purple.withAlpha (0.2f);
//    juce::Colour SOLO_SQUARE_COLOUR = juce::Colours::purple.withAlpha (0.3f);
    juce::Colour HOVER_SQUARE_COLOUR = juce::Colours::black.withAlpha (0.2f);
    
    // Solo'd square
    std::set<std::pair<int, int>> soloSquareCoords;
    std::optional<std::pair<int, int>> hoverSquareCoords;
    std::optional<std::pair<int, int>> mouseDownCoords;
    
    int soloCounter = 0;
    int soloMax = 80;
    bool isSolod = true;
    bool hasSelection = false; // if there is an active soloSquareCoords selection
};
