/*
  ==============================================================================

    AudioPlayerComponent.cpp
    Created: 10 Feb 2025 11:55:03pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "AudioPlayerComponent.h"

AudioPlayerComponent::AudioPlayerComponent()
{
    addAndMakeVisible (uploadFileButton);
    addAndMakeVisible (playButton);
    uploadFileButton.onClick = [this] {
        if (listener != nullptr)
        {
            openButtonClicked();
        }
    };
    
    playButton.onClick = [this] {
        if (listener != nullptr)
        {
            
            playButton.setEnabled (false);
        }
    };
}

AudioPlayerComponent::~AudioPlayerComponent()
{
    
}

void AudioPlayerComponent::paint (juce::Graphics& g)
{
    
}

void AudioPlayerComponent::resized()
{
    Layout layout (getBounds().withX (0).withY (0), 0.0f);
    layout.addRow ({ Space (&uploadFileButton) });
    layout.addRow ({ Space (&playButton) });
    layout.updateComponentBounds();
}

void AudioPlayerComponent::setListener (AudioPlayerComponentListener* listener)
{
    this->listener = listener;
    listener->addAsListener (this);
}

void AudioPlayerComponent::audioFilePlayingChanged (bool isPlaying)
{
    if (isPlaying)
    {
        playButton.setEnabled (true);
        playButton.setButtonText ("Stop");
    }
    else
    {
        playButton.setEnabled (true);
        playButton.setButtonText ("Play File");
    }
}

void AudioPlayerComponent::openButtonClicked()
{
//    std::unique_ptr<juce::FileChooser> chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
//                                                   juce::File{},
//                                                   "*.wav");                     // [7]
//    auto chooserFlags = juce::FileBrowserComponent::openMode
//                      | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)     // [8]
//    {
//        auto file = fc.getResult();
//        if (listener != nullptr)
//            listener->setFile (file);
//    });
}
