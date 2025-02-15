/*
  ==============================================================================

    CalibrationView.h
    Created: 22 Dec 2024 8:38:53pm
    Author:  Tyler Gee

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "BuildableComponent.h"
#include "Layout.h"
#include "Listeners.h"
#include "ArchetypeBar.h"
#include "GlyphGridView.h"
#include "CheckerboardView.h"

// This provides a UI for glyph calibration. It includes  a view that lets you drag and move around glyphs, a view that lets you add glyphs from a list, and a view with settings that impact playback.
class CalibrationView  : public BuildableComponent,
                         public juce::ComboBox::Listener,
                         public juce::Timer
{
public:
    CalibrationView();
    ~CalibrationView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (CheckerboardViewListener* listener);
    void setCalibrationListener (CalibrationListener* calibrationListener);
    //    void setDataSource (GlyphViewDataSource* dataSource);
    void setDataSource (CheckerboardViewDataSource* dataSource);
    
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
    void timerCallback() override;
    
private:
    void updateSpeakerButton();
    void updatePlayPauseButton(); // updates the images of playPauseButton based on isPlaying
    void updatePlayer(); // updates prev next buttons based on availability of prev and next
    
    CalibrationListener* calibrationListener = nullptr;
    CheckerboardViewDataSource* dataSource = nullptr;
    
    // Glyph View
    //    GlyphGridView glyphGridView;
    CheckerboardView checkerboardView;
    
    // Archetype Bar
    juce::Viewport archetypeViewport;
    ArchetypeBar archetypeBar;
    
    // Calibration settings
//    juce::Slider speedSlider;
//    juce::Label speedLabel;
//    juce::Slider bandwidthSlider;
//    juce::Label bandwidthLabel;
//    juce::Slider volumeSlider;
//    juce::Label volumeLabel;
//    juce::Slider sharpnessSlider;
//    juce::Label sharpnessLabel;
//    juce::TextButton playButton { "Play" };
//    juce::TextButton polarityButton { "Toggle Black/White" };
//    juce::TextButton autoPolarityButton { "Auto Toggle" };
//    juce::TextButton iirButton { "IIR" };
//    juce::TextButton updateFilterButton { "Update" };
//    juce::ComboBox qualityComboBox;
//    juce::ComboBox scalingComboBox;
//    juce::ComboBox erbComboBox;
//    juce::ComboBox pinkNoiseBox;
//    
//    juce::ComboBox resolutionComboBox;
//    juce::ComboBox isCascadingBox;
//    juce::Slider densitySlider;
//    juce::Label densityLabel;
//    juce::Slider strokeOverlapSlider;
//    juce::Label strokeOverlapLabel;
//    juce::Slider dotOverlapSlider;
//    juce::Label dotOverlapLabel;
//    juce::Slider rampLengthSlider;
//    juce::Label rampLengthLabel;
    
    // Play button stuff
    juce::Image playImage = juce::ImageFileFormat::loadFrom (BinaryData::PlayButtonIcon_png, BinaryData::PlayButtonIcon_pngSize);
    juce::Image pauseImage = juce::ImageFileFormat::loadFrom (BinaryData::PauseButtonIcon_png, BinaryData::PauseButtonIcon_pngSize);
    juce::Image prevImage = juce::ImageFileFormat::loadFrom (BinaryData::PrevButtonIcon_png, BinaryData::PrevButtonIcon_pngSize);
    juce::Image nextImage = juce::ImageFileFormat::loadFrom (BinaryData::NextButtonIcon_png, BinaryData::NextButtonIcon_pngSize);
    juce::Image speakerImage = juce::ImageFileFormat::loadFrom (BinaryData::SpeakerIcon_png, BinaryData::SpeakerIcon_pngSize);
    juce::Image mutedImage = juce::ImageFileFormat::loadFrom (BinaryData::MuteIcon_png, BinaryData::MuteIcon_pngSize);
    
    juce::Label titleLabel;
    juce::ImageButton playPauseButton;
    juce::ImageButton prevButton; // goes to the previous "song" (e.g. the previous checkerboard)
    juce::ImageButton nextButton; // goes to the next "song" (e.g. the next checkerboard)
    juce::ImageButton speakerButton; // a button, but for now, doesn't do anything
    juce::Slider noiseVolumeSlider;
    
    bool isMuted = false;
    float currVolume = 0.0f;
    bool isPlaying = false;
    bool isIIR = true;
    bool isFIRFilterUpdated = false;
    bool isAutoToggling = false;
};

