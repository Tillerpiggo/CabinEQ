/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "PlaybackManager.h"
#include "CabinEqProfileManager.h"

//==============================================================================
/**
*/
class CabinEqAudioProcessor  : public juce::AudioProcessor
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void didLoadData() = 0;
    };
    
    //==============================================================================
    CabinEqAudioProcessor();
    ~CabinEqAudioProcessor() override;

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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    
    void setIsProcessing (bool isProcessing);
    void setBypassBalance (float balance);
    void setVolume (float volume);
    void setMutedGens (std::vector<bool> mutedGens);
    
    // Calibration noises
    void startNoisePatternAt (int id, juce::String profileName);
    void updateNoisePatternAt (int id, juce::String profileName);
    void stopNoisePattern();
    void setNoisePatternSolo (bool solo);
    
    // Profiles
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void renameProfile (juce::String profileName, juce::String newProfileName);
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEqProfile>> getProfileNamed (juce::String profileName) const;
    std::vector<Band> getBands (juce::String profileName);
    
    std::optional<juce::String> getLastSelectedProfileName();
    void setLastSelectedProfileName (juce::String profileName);
    
    // Setting bands
    int addBand (const float freq, const float ampl, const float bandwidth, juce::String profileName);
    void updateBand (const int id, const float freq, const float ampl, const float bandwidth, juce::String profileName);
    void removeBand (const int id, juce::String profileName);
    
    
    // Listener
    void addListener (Listener* listener);
    void removeListener();

private:
    std::optional<std::reference_wrapper<CabinEqProfile>> profileNamed (juce::String profileName) const; // returns the current profile. Crashes if currentProfileId doesn't match an existing profile.

    PlaybackManager playbackManager;
    CabinEqProfileManager cabinEqProfileManager;
    
    juce::dsp::ProcessSpec spec;
    
    std::vector<Listener*> listeners;
    bool hasLoadedData = false;
    juce::String currProfileName { "" };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqAudioProcessor)
};
