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
    // Checkerboard view
    addAndMakeVisible (checkerboardView);
    
    // Setup new player component
    // Title
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (juce::Font (juce::FontOptions (20, juce::Font::bold)));
    titleLabel.setJustificationType (juce::Justification::centred);
    
    // Play button
    addAndMakeVisible (playPauseButton);
    updatePlayPauseButton();
    playPauseButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            isPlaying = ! isPlaying;
            calibrationListener->setIsPlaying (isPlaying);
            updatePlayPauseButton();
        }
    };
    
    // Prev + next buttons
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);
    prevButton.setImages (false, true, true, prevImage, 1.0f, juce::Colours::white.withAlpha (0.0f), prevImage, 1.0f, juce::Colours::black.withAlpha (0.1f), prevImage, 1.0f, juce::Colours::black.withAlpha (0.2f));
    nextButton.setImages (false, true, true, nextImage, 1.0f, juce::Colours::white.withAlpha (0.0f), nextImage, 1.0f, juce::Colours::black.withAlpha (0.1f), nextImage, 1.0f, juce::Colours::black.withAlpha (0.2f));
    updatePlayer();
    prevButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->goToPrev();
            checkerboardView.updateCheckerboard();
            updatePlayer();
        }
    };
    nextButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->goToNext();
            checkerboardView.updateCheckerboard();
            updatePlayer();
        }
    };
    
    // Volume slider
    addAndMakeVisible (speakerButton);
    updateSpeakerButton();
    addSlider (&noiseVolumeSlider, -120.0f, 12.0f, 0.0f);
    
    speakerButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            isMuted = ! isMuted;
            updateSpeakerButton();
            
            if (isMuted)
            {
                calibrationListener->setCalibrationVolume (-120.0f);
                currVolume = noiseVolumeSlider.getValue();
                noiseVolumeSlider.setValue (-120.0f);
            }
            else
            {
                calibrationListener->setCalibrationVolume (currVolume);
                noiseVolumeSlider.setValue (currVolume);
            }
        }
    };
    noiseVolumeSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    noiseVolumeSlider.onValueChange = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setCalibrationVolume (noiseVolumeSlider.getValue());
            isMuted = noiseVolumeSlider.getValue() == -120.0f;
            updateSpeakerButton();
        }
    };
    
    startTimer (1000);
    
    // Calibration setting components
//    addSliderAndLabel (&speedSlider, &speedLabel, "Speed", 0.1f, 5.0f, 1.0f);
//    speedSlider.setSkewFactorFromMidPoint (1.0f);
//    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.1f, 4.0f, 2.5f);
//    addSliderAndLabel (&volumeSlider, &volumeLabel, "Noise Volume", -24.0f, 24.0f, 0.0f);
////    addSliderAndLabel (&sharpnessSlider, &sharpnessLabel, "Sharpness", 0.5f, 1.0f, 0.8f);
//    addButton (&playButton);
//    addButton (&polarityButton);
//    addButton (&autoPolarityButton);
    
//    addAndMakeVisible (resolutionComboBox);
//    int minResolution = 2;
//    int maxResolution = 8;
//    for (int i = minResolution; i <= maxResolution; ++i)
//    {
//        resolutionComboBox.addItem (std::to_string (i) + "x" + std::to_string (i), i);
//    }
//    resolutionComboBox.setSelectedId (2); // start out at resolution 2
//    resolutionComboBox.addListener (this);
//    
//    addAndMakeVisible (scalingComboBox);
//    scalingComboBox.addItem ("Logarithmic", 1);
//    scalingComboBox.addItem ("Bark", 2);
//    scalingComboBox.setSelectedId (1);
//    scalingComboBox.addListener (this);
//    addAndMakeVisible (erbComboBox);
//    erbComboBox.addItem ("Uniform", 1);
//    erbComboBox.addItem ("ERB", 2);
//    erbComboBox.setSelectedId (1);
//    erbComboBox.addListener (this);
//    addAndMakeVisible (pinkNoiseBox);
//    pinkNoiseBox.addItem ("Cabin Noise (-4.5 dB/oct)", 1);
//    pinkNoiseBox.addItem ("Pink Noise (-3.0 dB/oct)", 2);
//    pinkNoiseBox.setSelectedId (2);
//    pinkNoiseBox.addListener (this);
//    addAndMakeVisible (isCascadingBox);
//    isCascadingBox.addItem ("Sweeping", 1);
//    isCascadingBox.addItem ("Cascading", 2);
//    isCascadingBox.setSelectedId (1);
//    isCascadingBox.addListener (this);
//    
//    addButton (&iirButton);
//    addButton (&updateFilterButton);
//    
//    addAndMakeVisible (qualityComboBox);
//    qualityComboBox.addItem ("Economy", 10);
//    qualityComboBox.addItem ("Good", 14);
//    qualityComboBox.addItem ("Ultra", 18);
//    qualityComboBox.addListener (this);
//    qualityComboBox.setSelectedId (14);
//    
//    addSliderAndLabel (&densitySlider, &densityLabel, "Density", 2.0f, 40.0f, 4.0f);
//    addSliderAndLabel (&strokeOverlapSlider, &strokeOverlapLabel, "Stroke Overlap", 0.0f, 1.0f, 0.2f);
//    addSliderAndLabel (&dotOverlapSlider, &dotOverlapLabel, "Dot Overlap", 0.0f, 1.0f, 0.2f);
////    addSliderAndLabel (&rampLengthSlider, &rampLengthLabel, "Sharpness", 0.0f, 0.5f, 0.2f);
//    
//    // Slider actions
//    addSliderAction (&speedSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//            calibrationListener->setSpeedFactor (speedSlider.getValue());
//    });
//    addSliderAction (&bandwidthSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setBandwidth (bandwidthSlider.getValue());
//            // TODO: propogate visual change to the glyph view
//        }
//    });
//    addSliderAction (&volumeSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setCalibrationVolume (volumeSlider.getValue());
////            glyphGridView.updateGlyphs();
//        }
//    });
//    addSliderAction (&sharpnessSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setCheckerboardSharpness (sharpnessSlider.getValue());
//        }
//    });
//    addSliderAction (&densitySlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setDensity ((int) densitySlider.getValue());
//            glyphGridView.updateGlyphs();
//        }
//    });
//    addSliderAction (&strokeOverlapSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setStrokeOverlap (strokeOverlapSlider.getValue());
//            glyphGridView.updateGlyphs();
//        }
//    });
//    addSliderAction (&dotOverlapSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setDotOverlap (dotOverlapSlider.getValue());
//            glyphGridView.updateGlyphs();
//        }
//    });
//    addSliderAction (&rampLengthSlider, [this](juce::Slider*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->setRampLength (rampLengthSlider.getValue());
//            glyphGridView.updateGlyphs();
//        }
//    });
    
//    // Button actions
//    addButtonAction (&playButton, [this](juce::Button*) {
//        isPlaying = ! isPlaying;
//        if (calibrationListener != nullptr)
//            calibrationListener->setIsPlaying (isPlaying);
//        playButton.setButtonText (isPlaying ? "Pause" : "Play");
//        checkerboardView.updateIsPlaying();
////        glyphGridView.updateIsPlaying();
//    });
//    addButtonAction (&polarityButton, [this](juce::Button*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->toggleCheckerboardPolarity();
//            checkerboardView.updateCheckerboard();
//        }
//    });
//    addButtonAction (&autoPolarityButton, [this](juce::Button*) {
//        if (calibrationListener != nullptr)
//        {
//            isAutoToggling = ! isAutoToggling;
//            if (isAutoToggling)
//            {
//                startTimer (1000);
//                autoPolarityButton.setButtonText ("Stop Auto Toggling");
//            }
//            else
//            {
//                stopTimer();
//                autoPolarityButton.setButtonText ("Auto Toggle");
//            }
//        }
//    });
//    addButtonAction (&iirButton, [this](juce::Button*) {
//        isIIR = ! isIIR;
//        if (calibrationListener != nullptr)
//            calibrationListener->setIIR (isIIR);
//        iirButton.setButtonText (isIIR ? "IIR" : "FIR");
//    });
//
//    addButtonAction (&updateFilterButton, [this](juce::Button*) {
//        if (calibrationListener != nullptr)
//        {
//            calibrationListener->updateFIRFilter();
//        }
//    });
//
//    
//    // Glyph Grid View
////    addAndMakeVisible (glyphGridView);
//    // Checkerboard View
//    addAndMakeVisible (checkerboardView);
//    
//    // Archetype Bar
//    addAndMakeVisible (archetypeViewport);
//    addAndMakeVisible (archetypeBar);
////    archetypeViewport.setViewedComponent (&archetypeBar);
//    archetypeViewport.setScrollBarsShown (true, false);
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
    glyphLayout.addRow ({ Space (&checkerboardView ) });
    glyphLayout.updateComponentBounds();
    
    Layout playerLayout (localBounds.withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
    playerLayout.addRow ({ Space() });
    playerLayout.addRow ({ Space (&titleLabel) });
    playerLayout.addRow ({ Space(), Space (&prevButton, 40.0f), Space (&playPauseButton, 80.0f), Space (&nextButton, 40.0f), Space() } );
    playerLayout.addRow ({ Space() }, 8.0f);
    playerLayout.addRow ({ Space(), Space (&speakerButton, 16.0f), Space (&noiseVolumeSlider), Space() }, 20.0f);
    playerLayout.addRow ({ Space() });
    playerLayout.updateComponentBounds();
    
    // Archetype sidebar
//    Layout archetypeBarLayout (localBounds.withTrimmedRight (sidebarWidth).withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
//    archetypeBarLayout.addRow ({ Space (&archetypeBar) });
//    archetypeBarLayout.updateComponentBounds();
    
//    // Settings section
//    Layout settingsLayout (localBounds.withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
////    settingsLayout.addRow ({ Space (80), Space (&speedSlider) });
////    settingsLayout.addRow ({ Space (80), Space (&bandwidthSlider) });
//    float volumeLabelWidth = volumeLabel.getFont().getStringWidth (volumeLabel.getText());
////    float sharpnessLabelWidth = sharpnessLabel.getFont().getStringWidth (sharpnessLabel.getText());
//    settingsLayout.addRow ({ Space (volumeLabelWidth), Space (&volumeSlider) });
////    settingsLayout.addRow ({ Space (sharpnessLabelWidth), Space (&sharpnessSlider) });
//    settingsLayout.addRow ({ Space (&resolutionComboBox) });
//    settingsLayout.addRow ({ Space (&polarityButton), Space (&autoPolarityButton, 100) });
//    settingsLayout.addRow ({ Space (&isCascadingBox) });
//    if (isCascadingBox.getSelectedId() == 2)
//    {
//        settingsLayout.addRow ({ Space (80), Space (&densitySlider) });
//        settingsLayout.addRow ({ Space (80), Space (&strokeOverlapSlider) });
//        settingsLayout.addRow ({ Space (80), Space (&dotOverlapSlider) });
//        settingsLayout.addRow ({ Space (80), Space (&rampLengthSlider) });
//        densitySlider.setVisible (true);
//        strokeOverlapSlider.setVisible (true);
//        dotOverlapSlider.setVisible (true);
//        rampLengthSlider.setVisible (true);
//    }
//    else
//    {
//        densitySlider.setVisible (false);
//        strokeOverlapSlider.setVisible (false);
//        dotOverlapSlider.setVisible (false);
//        rampLengthSlider.setVisible (false);
//    }
//    settingsLayout.addRow ({ Space (&pinkNoiseBox) });
//    settingsLayout.addRow ({ Space (&scalingComboBox), Space (&erbComboBox), Space (&pinkNoiseBox) });
//    settingsLayout.addRow ({ Space (&iirButton), Space (&qualityComboBox), Space (&updateFilterButton) });
//    settingsLayout.addRow ({ Space (&playButton) });
//    settingsLayout.updateComponentBounds();
}

//void CalibrationView::setListener (GlyphViewListener* listener)
//{
////    glyphGridView.setListener (listener);
//}

void CalibrationView::setListener (CheckerboardViewListener* listener)
{
    checkerboardView.setListener (listener);
//    this->checkerboardViewListener = listener;
}

void CalibrationView::setCalibrationListener (CalibrationListener* calibrationListener)
{
    this->calibrationListener = calibrationListener;
    updatePlayer();
}

//void CalibrationView::setDataSource (GlyphViewDataSource* dataSource)
//{
//    glyphGridView.setDataSource (dataSource);
//    archetypeBar.setDataSource (dataSource);
//}

void CalibrationView::setDataSource (CheckerboardViewDataSource* dataSource)
{
    checkerboardView.setDataSource (dataSource);
    this->dataSource = dataSource;
    updatePlayer();
}

void CalibrationView::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (calibrationListener == nullptr)
    {
        return;
//        calibrationListener->setFIRQuality (qualityComboBox.getSelectedId());
    }
    
//    if (comboBoxThatHasChanged == &scalingComboBox)
//    {
//        calibrationListener->setBarkScaling (scalingComboBox.getSelectedId() == 2);
//    }
//    else if (comboBoxThatHasChanged == &erbComboBox)
//    {
//        calibrationListener->setERBScaling (erbComboBox.getSelectedId() == 2);
//    }
//    else if (comboBoxThatHasChanged == &pinkNoiseBox)
//    {
//        calibrationListener->setPinkNoise (pinkNoiseBox.getSelectedId() == 2);
//    }
//    else if (comboBoxThatHasChanged == &isCascadingBox)
//    {
//        calibrationListener->setIsCascading (isCascadingBox.getSelectedId() == 2);
//        resized();
////        glyphGridView.updateGlyphs();
//    }
//    else if (comboBoxThatHasChanged == &resolutionComboBox)
//    {
////        calibrationListener->setCheckerboardResolution (resolutionComboBox.getSelectedId());
//        checkerboardView.updateCheckerboard();
//        resized();
//    }
}

void CalibrationView::timerCallback()
{
//    std::cout << "timer callback" << std::endl;
    if (calibrationListener != nullptr && isPlaying)
    {
        calibrationListener->toggleCheckerboardPolarity();
        checkerboardView.updateCheckerboard();
    }
}

void CalibrationView::updateSpeakerButton()
{
    juce::Image currSpeakerImage = isMuted ? mutedImage : speakerImage;
    speakerButton.setImages (false, true, true, currSpeakerImage, 1.0f, juce::Colours::white.withAlpha (0.1f), currSpeakerImage, 1.0f, juce::Colours::white.withAlpha (0.2f), currSpeakerImage, 1.0f, juce::Colours::white.withAlpha (1.0f));
}

void CalibrationView::updatePlayPauseButton()
{
    juce::Image buttonImage = isPlaying ? pauseImage : playImage;
    playPauseButton.setImages (false, true, true, buttonImage, 1.0f, juce::Colours::white.withAlpha (0.0f), buttonImage, 1.0f, juce::Colours::black.withAlpha (0.1f), buttonImage, 1.0f, juce::Colours::black.withAlpha (0.2f));
}

void CalibrationView::updatePlayer()
{
    if (calibrationListener == nullptr || dataSource == nullptr)
    {
        prevButton.setEnabled (false);
        nextButton.setEnabled (false);
        titleLabel.setText ("No Audio Selected", juce::NotificationType::dontSendNotification);
        return;
    }
    bool hasPrev = calibrationListener->hasPrev();
    bool hasNext = calibrationListener->hasNext();
    prevButton.setEnabled (calibrationListener->hasPrev());
    prevButton.setAlpha (hasPrev ? 1.0f : 0.8f);
    nextButton.setEnabled (calibrationListener->hasNext());
    nextButton.setAlpha (hasNext ? 1.0f : 0.8f);
    titleLabel.setText (dataSource->getCheckerboard().getTitle(), juce::NotificationType::dontSendNotification);
    
}
