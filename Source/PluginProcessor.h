/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "ArbitraryResponseFilter.h"
#include "SliderCalibrationManager.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    StartupMVPAudioProcessor();
    ~StartupMVPAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    const std::pair<juce::String, double>& getNoteData(int index) const;
    int getNoteDataSize() const;
    
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout (int numPoints);
    juce::AudioProcessorValueTreeState parameters;
    
    void applyCurve();
    void toggleBypass();
    void setBypassBalance (float balance);
    
    SliderCalibrationManager& getSliderCalibrationManager()
    {
        return sliderCalibrationManager;
    }
 
private:
    static const int FFT_SIZE = 12;
    
    bool isBypassed = false;
    bool isCalibrating = false;

    ArbitraryResponseFilter gainFilter;
    SliderCalibrationManager sliderCalibrationManager;
    
    juce::dsp::ProcessSpec spec;
    juce::dsp::Gain<float> dryGainProcessor;
    juce::dsp::Gain<float> wetGainProcessor;
    
    // Hacky solution to fix gainFilter bug
    int sampleRate;
    int samplesPerBlock;
    bool hasPreparedFilter = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessor)
};
