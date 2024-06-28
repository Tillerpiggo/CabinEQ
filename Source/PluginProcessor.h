/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SetPointManager.h"
#include "Curve.h"
#include "Sequencer.h"
#include "ArbitraryResponseFilter.h"
#include "BalanceArbitraryResponseFilter.h"
#include "RandomCalibrationManager.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessor  : public juce::AudioProcessor, juce::Timer, public juce::Slider::Listener
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
    void timerCallback() override;
    void sliderValueChanged (juce::Slider *slider) override;
    void sliderDragStarted (juce::Slider *slider) override;
    void sliderDragEnded (juce::Slider *slider) override;
    
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout (int numPoints);
    juce::AudioProcessorValueTreeState parameters;
    
    const SetPointManager& getSetPointManager() const { return setPointLayout; } // TODO: Is this correct reference semantics for C++? I feel like I'm doing something wrong
    const Curve& getCurve() const { return calibrationManager.getGainCurve(); }
    int goToPrevInterval() { return calibrationManager.goToPrevInterval(); }
    int goToNextInterval() { return calibrationManager.goToNextInterval(); }
    void toggleBypass();

private:
    static const int FFT_SIZE = 16;
    
    std::vector<std::pair<juce::String, double>> noteData;
    
    RandomCalibrationManager calibrationManager;
    
    int selectedSliderIndex;
    bool isSlidingGainSlider;
    bool isSlidingPanSlider;
    bool isSlidingPhaseSlider;
    
    bool isBypassed;
    
    SetPointManager setPointLayout;
//    Curve curve;
//    Curve balanceCurve;
    ArbitraryResponseFilter gainFilter;
    ArbitraryResponseFilter gainFilter2;
//    BalanceArbitraryResponseFilter balanceFilter;
    juce::dsp::Gain<float> gainProcessor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessor)
};
