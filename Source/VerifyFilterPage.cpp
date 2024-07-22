/*
  ==============================================================================

    VerifyFilterPage.cpp
    Created: 20 Jul 2024 3:25:29pm
    Author:  Tyler Gee

  ==============================================================================
*/

#include "VerifyFilterPage.h"

VerifyFilterPage::VerifyFilterPage (StartupMVPAudioProcessor& p) : processor (p)
{
    tone1FreqSlider.setRange(20.0, 20000.0, 1.0);
    tone1FreqSlider.setSkewFactorFromMidPoint(1000.0);
    tone1FreqSlider.addListener(this);
    addAndMakeVisible(tone1FreqSlider);

    tone1VolSlider.setRange(-48.0, 48.0, 0.1);
    tone1VolSlider.addListener(this);
    addAndMakeVisible(tone1VolSlider);

    tone2FreqSlider.setRange(20.0, 20000.0, 1.0);
    tone2FreqSlider.setSkewFactorFromMidPoint(1000.0);
    tone2FreqSlider.addListener(this);
    addAndMakeVisible(tone2FreqSlider);

    tone2VolSlider.setRange(-48.0, 48.0, 0.1);
    tone2VolSlider.addListener(this);
    addAndMakeVisible(tone2VolSlider);

    filterVolumeSlider.setRange(-48.0, 48.0, 0.1);
    filterVolumeSlider.addListener(this);
    addAndMakeVisible(filterVolumeSlider);
    
    togglePlayingButton.addListener(this);
    addAndMakeVisible(togglePlayingButton);

    toggleFilterButton.addListener(this);
    addAndMakeVisible(toggleFilterButton);
}

VerifyFilterPage::~VerifyFilterPage()
{
    tone1FreqSlider.removeListener(this);
    tone1VolSlider.removeListener(this);
    tone2FreqSlider.removeListener(this);
    tone2VolSlider.removeListener(this);
    filterVolumeSlider.removeListener(this);
    togglePlayingButton.removeListener(this);
    toggleFilterButton.removeListener(this);
}

void VerifyFilterPage::resized()
{
    // Layout components in a vertical stack
    auto area = getLocalBounds();
    auto sliderHeight = 40;

    tone1FreqSlider.setBounds(area.removeFromTop(sliderHeight));
    tone1VolSlider.setBounds(area.removeFromTop(sliderHeight));
    tone2FreqSlider.setBounds(area.removeFromTop(sliderHeight));
    tone2VolSlider.setBounds(area.removeFromTop(sliderHeight));
    filterVolumeSlider.setBounds(area.removeFromTop(sliderHeight));
    togglePlayingButton.setBounds(area.removeFromTop(sliderHeight));
    toggleFilterButton.setBounds(area.removeFromTop(sliderHeight));
}

void VerifyFilterPage::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
}

void VerifyFilterPage::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &tone1FreqSlider)
    {
        processor.getSliderCalibrationManager().setTone1Freq(slider->getValue());
    }
    else if (slider == &tone1VolSlider)
    {
        processor.getSliderCalibrationManager().setTone1Vol(slider->getValue());
    }
    else if (slider == &tone2FreqSlider)
    {
        processor.getSliderCalibrationManager().setTone2Freq(slider->getValue());
    }
    else if (slider == &tone2VolSlider)
    {
        processor.getSliderCalibrationManager().setTone2Vol(slider->getValue());
    }
    else if (slider == &filterVolumeSlider)
    {
        processor.getSliderCalibrationManager().setFilterGain(slider->getValue());
    }
}

void VerifyFilterPage::sliderDragStarted (juce::Slider *slider)
{
    // Do nothing
}

void VerifyFilterPage::sliderDragEnded (juce::Slider *slider)
{
    // Do nothing
}

void VerifyFilterPage::buttonClicked (juce::Button *button)
{
    if (button == &togglePlayingButton)
    {
        isTesting = ! isTesting;
        processor.getSliderCalibrationManager().setIsTesting (isTesting);
        
        if (isTesting)
        {
            togglePlayingButton.setButtonText ("Stop Playing");
        }
        else
        {
            togglePlayingButton.setButtonText ("Start Playing");
        }
    }
    else if (button == &toggleFilterButton)
    {
        isFilterEnabled = ! isFilterEnabled;
        processor.getSliderCalibrationManager().setIsFilterEnabled (isFilterEnabled);
        
        if (isFilterEnabled)
        {
            toggleFilterButton.setButtonText ("Disable Filter");
        }
        else
        {
            toggleFilterButton.setButtonText ("Enable Filter");
        }
    }
}
