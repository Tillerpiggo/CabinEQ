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
    speedSlider.setSkewFactorFromMidPoint (1.0f);
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 1.0f, 4.0f, 2.5f);
    addSliderAndLabel (&volumeSlider, &volumeLabel, "Volume", -24.0f, 24.0f, 0.0f);
    addButton (&playButton);
    
    addAndMakeVisible (scalingComboBox);
    scalingComboBox.addItem ("Logarithmic", 1);
    scalingComboBox.addItem ("Bark", 2);
    scalingComboBox.setSelectedId (1);
    scalingComboBox.addListener (this);
    addAndMakeVisible (erbComboBox);
    erbComboBox.addItem ("Uniform", 1);
    erbComboBox.addItem ("ERB", 2);
    erbComboBox.setSelectedId (1);
    erbComboBox.addListener (this);
    addAndMakeVisible (pinkNoiseBox);
    pinkNoiseBox.addItem ("Cabin Noise", 1);
    pinkNoiseBox.addItem ("Pink Noise", 2);
    pinkNoiseBox.setSelectedId (1);
    pinkNoiseBox.addListener (this);
    
//    addButton (&iirButton);
//    addButton (&updateFilterButton);
//    
//    addAndMakeVisible (qualityComboBox);
//    qualityComboBox.addItem ("Economy", 10);
//    qualityComboBox.addItem ("Good", 14);
//    qualityComboBox.addItem ("Ultra", 18);
//    qualityComboBox.addListener (this);
//    qualityComboBox.setSelectedId (14);
    
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
    addSliderAction (&volumeSlider, [this](juce::Slider*) {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setCalibrationVolume (volumeSlider.getValue());
        }
    });
    
    // Button actions
    addButtonAction (&playButton, [this](juce::Button*) {
        isPlaying = ! isPlaying;
        if (calibrationListener != nullptr)
            calibrationListener->setIsPlaying (isPlaying);
        playButton.setButtonText (isPlaying ? "Pause" : "Play");
    });
    addButtonAction (&iirButton, [this](juce::Button*) {
        isIIR = ! isIIR;
        if (calibrationListener != nullptr)
            calibrationListener->setIIR (isIIR);
        iirButton.setButtonText (isIIR ? "IIR" : "FIR");
    });

    addButtonAction (&updateFilterButton, [this](juce::Button*) {
        if (calibrationListener != nullptr)
        {
            calibrationListener->updateFIRFilter();
        }
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
    settingsLayout.addRow ({ Space (80), Space (&volumeSlider) });
//    settingsLayout.addRow ({ Space (&scalingComboBox), Space (&erbComboBox), Space (&pinkNoiseBox) });
//    settingsLayout.addRow ({ Space (&iirButton), Space (&qualityComboBox), Space (&updateFilterButton) });
    settingsLayout.addRow ({ Space (&playButton) });
    settingsLayout.updateComponentBounds();
}

void CalibrationView::setListener (GlyphViewListener* listener)
{
    glyphGridView.setListener (listener);
}

void CalibrationView::setCalibrationListener (CalibrationListener* calibrationListener)
{
    this->calibrationListener = calibrationListener;
}

void CalibrationView::setDataSource (GlyphViewDataSource* dataSource)
{
    glyphGridView.setDataSource (dataSource);
    archetypeBar.setDataSource (dataSource);
}

void CalibrationView::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (calibrationListener == nullptr)
    {
        return;
//        calibrationListener->setFIRQuality (qualityComboBox.getSelectedId());
    }
    
    if (comboBoxThatHasChanged == &scalingComboBox)
    {
        calibrationListener->setBarkScaling (scalingComboBox.getSelectedId() == 2);
    }
    else if (comboBoxThatHasChanged == &erbComboBox)
    {
        calibrationListener->setERBScaling (erbComboBox.getSelectedId() == 2);
    }
    else if (comboBoxThatHasChanged == &pinkNoiseBox)
    {
        calibrationListener->setPinkNoise (pinkNoiseBox.getSelectedId() == 2);
    }
}
