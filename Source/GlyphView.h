/*
  ==============================================================================

    GlyphView.h
    Created: 14 Nov 2024 3:40:07pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Glyph.h"
#include "Layout.h"
#include "BuildableComponent.h"
#include "AnimatedGlyph.h"
#include "UIConstants.h"
#include "Listeners.h"

// This provides a UI to play a Glyph and see the visuals, which includes a speed slider, a bandwidth slider, a frequency range (TODO) and a panning range (TODO). Also includes next/prev buttons to navigate between Glyphs.
class GlyphView  : public BuildableComponent,
                   public AnimatedGlyph::Listener,
                   public AnimatedGlyph::DataSource
{
public:
    GlyphView();
    ~GlyphView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (GlyphViewListener* listener);
    void setCalibrationListener (CalibrationListener* calibrationListener);
    void setDataSource (GlyphViewDataSource* dataSource);
    
    // AnimatedGlyph::Listener
    void setSizeFactor (float sizeFactor) override;
    void setCenterPos (juce::Point<float> centerPos) override;
    
    // AnimatedGlyph::DataSource
    float getSizeFactor() override;
    juce::Point<float> getCenterPos() override;
    float getCurrTime() override;
    
private:
    void updatePrevNextButtons();
    
    GlyphViewListener* listener = nullptr;
    CalibrationListener* calibrationListener = nullptr;
    GlyphViewDataSource* dataSource = nullptr;
    
    std::optional<Glyph> glyph;
    
    AnimatedGlyph animatedGlyph;
    juce::Slider speedSlider;
    juce::Label speedLabel;
    juce::Slider bandwidthSlider;
    juce::Label bandwidthLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton prevButton { "Prev" };
    juce::TextButton nextButton { "Next" };
    
    bool isPlaying = false;
};