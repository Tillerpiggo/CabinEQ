/*
  ==============================================================================

    CalibrationView.cpp
    Created: 22 Dec 2024 8:38:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationView.h"

CalibrationView::CalibrationView()
{
    // Calibration setting components
    addSliderAndLabel (&speedSlider, &speedLabel, "Speed", 0.1f, 5.0f, 1.0f);
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.1f, 4.0f, 1.0f);
    addButton (&playButton);
    
    // Slider actions
    addSliderAction (&speedSlider, [this](juce::Slider*) {
        if (calibrationListener != nullptr)
            calibrationListener->setSpeedFactor (speedSlider.getValue());
    });
    addSliderAction (&bandwidthSlider, [this](juce::Slider*) {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setBandwidth (bandwidthSlider.getValue());
            // TODO: propogate visual change to the glyph view
        }
    });
    
    // Button actions
    addButtonAction (&playButton, [this](juce::Button*) {
        isPlaying = ! isPlaying;
        if (calibrationListener != nullptr)
            calibrationListener->setIsPlaying (isPlaying);
        playButton.setButtonText (isPlaying ? "Pause" : "Play");
    });
    
    // Glyph Grid View
    addAndMakeVisible (glyphGridView);
    
    // Archetype Bar
    addAndMakeVisible (archetypeViewport);
    addAndMakeVisible (archetypeBar);
//    archetypeViewport.setViewedComponent (&archetypeBar);
    archetypeViewport.setScrollBarsShown (true, false);
}

CalibrationView::~CalibrationView()
{
}

void CalibrationView::paint (juce::Graphics& g)
{
    // Do nothing, let the subcomponents do the painting
}

void CalibrationView::resized()
{
    float sidebarWidth = 300.0f;
    float archetypeBarWidth = 100.0f;
    auto localBounds = getBounds().withX (0).withY (0);
    
    // Glyph View
    Layout glyphLayout (localBounds.withTrimmedRight (sidebarWidth + archetypeBarWidth), 8.0f);
    glyphLayout.addRow ({ Space (&glyphGridView) });
    glyphLayout.updateComponentBounds();
    
    // Archetype sidebar
    Layout archetypeBarLayout (localBounds.withTrimmedRight (sidebarWidth).withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
    archetypeBarLayout.addRow ({ Space (&archetypeBar) });
    archetypeBarLayout.updateComponentBounds();
    
    // Settings section
    Layout settingsLayout (localBounds.withTrimmedLeft (getWidth() - sidebarWidth), 8.0f);
    settingsLayout.addRow ({ Space (80), Space (&speedSlider) });
    settingsLayout.addRow ({ Space (80), Space (&bandwidthSlider) });
    settingsLayout.addRow ({ Space (&playButton) });
    settingsLayout.updateComponentBounds();
}

void CalibrationView::setListener (GlyphViewListener* listener)
{
    // TODO: forward to glyph view
}

void CalibrationView::setCalibrationListener (CalibrationListener* calibrationListener)
{
    this->calibrationListener = calibrationListener;
}

void CalibrationView::setDataSource (GlyphViewDataSource* dataSource)
{
    archetypeBar.setDataSource (dataSource);
    // TODO: forward to relevant views (glyph + archetype bar)
}
