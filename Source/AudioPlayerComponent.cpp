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
        
    };
    
    playButton.onClick = [this] {
        
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
    
}

void AudioPlayerComponent::setListener (AudioPlayerComponentListener* listener)
{
//    this->listener = listener;
}
