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

    // Glyph Grid View
    // addAndMakeVisible (glyphGridView); // Commented out

    // Archetype Bar
    // addAndMakeVisible (archetypeViewport); // Commented out
    // addAndMakeVisible (archetypeBar); // Commented out
    // archetypeViewport.setScrollBarsShown (true, false); // Commented out
    
    // Crossfeed Control
    addAndMakeVisible (crossfeedControl);

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
            musicList.updateSelectedRow();
            updatePlayer();
        }
    };
    nextButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->goToNext();
            checkerboardView.updateCheckerboard();
            musicList.updateSelectedRow();
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
    
    // Min Freq slider
    addSliderAndLabel (&minFreqSlider, &minFreqLabel, "Minimum Frequency", 20.0f, 150.0f, 20.0f);
    minFreqSlider.onValueChange = [this] {
        if (calibrationListener != nullptr)
            calibrationListener->setMinFreq (minFreqSlider.getValue());
    };

    // Glyph Sliders/Labels
    addSliderAndLabel (&speedSlider, &speedLabel, "Speed", 0.0f, 2.0f, 1.0f);
    addSliderAndLabel (&bandwidthSlider, &bandwidthLabel, "Bandwidth", 0.0f, 10.0f, 1.0f);
    addSliderAndLabel (&volumeSlider, &volumeLabel, "Volume", 0.0f, 1.0f, 0.5f);
    addAndMakeVisible (playButton);
    playButton.onClick = [this] {
        if (calibrationListener != nullptr)
        {
            isPlaying = ! isPlaying;
            calibrationListener->setIsPlaying (isPlaying);
            glyphGridView.updateGlyphs();
        }
    };
    speedSlider.onValueChange = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setSpeedFactor (speedSlider.getValue());
            glyphGridView.updateGlyphs();
        }
    };
    bandwidthSlider.onValueChange = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setBandwidth (bandwidthSlider.getValue());
            glyphGridView.updateGlyphs();
        }
    };
    volumeSlider.onValueChange = [this] {
        if (calibrationListener != nullptr)
        {
            calibrationListener->setGlyphVolume(volumeSlider.getValue());
            glyphGridView.updateGlyphs();
        }
    };
    startTimer (2000);
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
    float musicListWidth = 120.0f;
    float sidebarWidth = 300.0f;
    float archetypeBarWidth = 100.0f;
    auto localBounds = getBounds().withX (0).withY (0);
    
    // // Music List
    // Layout musicListLayout (localBounds.withTrimmedRight (getWidth() - musicListWidth), 4.0f);
    // musicListLayout.addRow ({ Space (&musicList) });
    // musicListLayout.updateComponentBounds();
    
     // Checkerboard View
//     Layout checkerboardLayout (localBounds.withTrimmedRight (sidebarWidth + archetypeBarWidth).withTrimmedLeft (musicListWidth), 8.0f);
//     checkerboardLayout.addRow ({ Space (&checkerboardView ) });
//     checkerboardLayout.updateComponentBounds();

    // Layout glyphGridViewLayout (localBounds.withTrimmedRight (sidebarWidth + archetypeBarWidth).withTrimmedLeft (musicListWidth), 8.0f);
    // glyphGridViewLayout.addRow ({ Space (&glyphGridView) });
    // glyphGridViewLayout.updateComponentBounds();
//
//    float sidebarWidth = 300.0f;
//    float archetypeBarWidth = 100.0f;
//    auto localBounds = getBounds().withX (0).withY (0);
    
// //    // Glyph View
//     Layout glyphLayout (localBounds.withTrimmedRight (sidebarWidth + archetypeBarWidth), 8.0f);
//     glyphLayout.addRow ({ Space (&glyphGridView) });
//     glyphLayout.updateComponentBounds();
// //    
//     // Archetype sidebar
//     Layout archetypeBarLayout (localBounds.withTrimmedRight (sidebarWidth).withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
//     archetypeBarLayout.addRow ({ Space (&archetypeBar) });
//     archetypeBarLayout.updateComponentBounds();

//     // Settings sidebar - use only the sidebarWidth portion on the right
//     Layout settingsLayout (localBounds.withTrimmedLeft (getWidth() - sidebarWidth), 8.0f);
//     settingsLayout.addRow ({ Space(&speedSlider), Space(&speedLabel) });
//     settingsLayout.addRow ({ Space(&bandwidthSlider), Space(&bandwidthLabel) });
//     settingsLayout.addRow ({ Space(&volumeSlider), Space(&volumeLabel) });
//     settingsLayout.addRow ({ Space(&playButton) });
//     settingsLayout.updateComponentBounds();

    Layout crossfeedLayout (localBounds.withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
    crossfeedLayout.addRow ({ Space(&crossfeedControl) });
    crossfeedLayout.updateComponentBounds();
    
//     Layout playerLayout (localBounds.withTrimmedLeft (getWidth() - (sidebarWidth + archetypeBarWidth)), 8.0f);
//     playerLayout.addRow ({ Space() });
//     playerLayout.addRow ({ Space (&titleLabel) });
//     playerLayout.addRow ({ Space(), Space (&prevButton, 40.0f), Space (&playPauseButton, 80.0f), Space (&nextButton, 40.0f), Space() } );
//     playerLayout.addRow ({ Space() }, 8.0f);
//     playerLayout.addRow ({ Space(), Space (&speakerButton, 16.0f), Space (&noiseVolumeSlider), Space() }, 20.0f);
//     playerLayout.addRow ({ Space() });
//     playerLayout.updateComponentBounds();
}

void CalibrationView::setListener (CheckerboardViewListener* listener)
{
    checkerboardView.setListener (listener);
//    this->checkerboardViewListener = listener;
}

void CalibrationView::setGlyphListener (GlyphViewListener* listener)
{
    glyphGridView.setListener (listener);
}

void CalibrationView::setCalibrationListener (CalibrationListener* calibrationListener)
{
    this->calibrationListener = calibrationListener;
    crossfeedControl.setListener(calibrationListener); // Set listener for crossfeed control
    updatePlayer();
}

void CalibrationView::setGlyphDataSource (GlyphViewDataSource* dataSource)
{
    glyphGridView.setDataSource (dataSource);
    archetypeBar.setDataSource (dataSource);
    this->glyphDataSource = dataSource;
}

void CalibrationView::setDataSource (CheckerboardViewDataSource* dataSource)
{
    checkerboardView.setDataSource (dataSource);
    musicList.setDataSource (dataSource);
    this->dataSource = dataSource;
    updatePlayer();
}

void CalibrationView::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (calibrationListener == nullptr)
    {
        return;
    }
}

void CalibrationView::selectedRow (int rowIdx)
{
    if (calibrationListener != nullptr)
    {
        calibrationListener->selectCheckerboardAtIdx (rowIdx);
        checkerboardView.updateCheckerboard();
        updatePlayer();
    }
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
