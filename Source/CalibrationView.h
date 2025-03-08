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
#include "CheckerboardView.h"
#include "MusicList2.h"
#include "GlyphGridView.h"
#include "ArchetypeBar.h"

// This provides a UI for glyph calibration. It includes  a view that lets you drag and move around glyphs, a view that lets you add glyphs from a list, and a view with settings that impact playback.
class CalibrationView  : public BuildableComponent,
                         public juce::ComboBox::Listener,
                         public MusicListListener,
                         public juce::Timer
{
public:
    CalibrationView();
    ~CalibrationView() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void setListener (CheckerboardViewListener* listener);
    void setGlyphListener (GlyphViewListener* listener);
    void setCalibrationListener (CalibrationListener* calibrationListener);
    void setGlyphDataSource (GlyphViewDataSource* dataSource);
    void setDataSource (CheckerboardViewDataSource* dataSource);
    
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
    void selectedRow (int rowIdx) override;
    
    void timerCallback() override;
    
private:
    void updateSpeakerButton();
    void updatePlayPauseButton(); // updates the images of playPauseButton based on isPlaying
    void updatePlayer(); // updates prev next buttons based on availability of prev and next
    
    CalibrationListener* calibrationListener = nullptr;
    CheckerboardViewDataSource* dataSource = nullptr;
    GlyphViewDataSource* glyphDataSource = nullptr;
    
    // Music list
    MusicList musicList;
    
    // Glyph View
    GlyphGridView glyphGridView;
    CheckerboardView checkerboardView;
    
    juce::Viewport archetypeViewport;
    ArchetypeBar archetypeBar;
    
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
    juce::Slider minFreqSlider;
    juce::Label minFreqLabel;

    // Glyph Sliders/Labels
    juce::Slider speedSlider;
    juce::Label speedLabel;
    juce::Slider bandwidthSlider;
    juce::Label bandwidthLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::TextButton playButton { "Play" };
    
    bool isMuted = false;
    float currVolume = 0.0f;
    bool isPlaying = false;
    bool isIIR = true;
    bool isFIRFilterUpdated = false;
    bool isAutoToggling = false;
};

