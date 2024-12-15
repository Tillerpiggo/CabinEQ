/*
  ==============================================================================

    NoiseGridView.h
    Created: 14 Dec 2024 5:04:28pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "NoiseSequenceGrid.h"
#include "Listeners.h"

class NoiseGridView  : public BuildableComponent,
                       public juce::Timer
{
public:
    NoiseGridView();
    ~NoiseGridView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    void mouseWheelMove (const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;
    
    void timerCallback() override;
    
    void setListener (NoiseGridViewListener* listener);
    void setDataSource (NoiseGridViewDataSource* dataSource);
    void removeListener();
    void removeDataSource();
    
private:
    void drawSquares (juce::Graphics& g);
    void drawSequences (juce::Graphics& g);
    std::pair<float, float> squareCoordsFromRowAndCol (int row, int col);
    void updateVisualConstants();
    
    juce::Rectangle<float> squareBoundsAtCoords (std::pair<int, int> coords);
    
    NoiseGridViewListener* listener = nullptr;
    NoiseGridViewDataSource* dataSource = nullptr;
    
    // Interaction variables
    std::vector<std::pair<int, int>> addingCoords;
    
    // Visual constants
    float xOffset;
    float yOffset;
    float squareSize;
    float padding = 10.0f;
};
