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

// This provides a UI to play a Glyph and see the visuals, which includes a speed slider, a bandwidth slider, a frequency range (TODO) and a panning range (TODO). Also includes next/prev buttons to navigate between Glyphs.
class GlyphView  : public BuildableComponent
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        
        virtual void setSpeed (float speedFactor) = 0;
        virtual void setBandwidth (float bandwidth) = 0;
        virtual void setIsPlaying (bool isPlaying) = 0;
        virtual void goToNext() = 0;
        virtual void goToPrev() = 0;
    };
    
    class DataSource
    {
    public:
        virtual ~DataSource() = default;
        
        virtual Glyph getCurrGlyph() = 0;
        virtual bool hasNext() = 0;
        virtual bool hasPrev() = 0;
        virtual float getCurrPlayingTime() = 0;
    };
    
    GlyphView();
    ~GlyphView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (Listener* listener);
    void setDataSource (DataSource* dataSource);
    
private:
    void updatePrevNextButtons();
    
    Listener* listener = nullptr;
    DataSource* dataSource = nullptr;
    
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
