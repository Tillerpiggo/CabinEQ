/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "PlaybackManager.h"
#include "CabinEQValueTreeManager.h"

//==============================================================================
/**
*/
class StartupMVPAudioProcessor  : public juce::AudioProcessor
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void didLoadData() = 0;
    };
    
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
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;
    
    void applyCurve (juce::String profileName);
    void applyCurves (juce::String leftCurveName, juce::String rightCurveName);
    void setIsProcessing (bool isProcessing);
    void setBypassBalance (float balance);
    
    std::optional<std::reference_wrapper<Curve>> getCurve (juce::String profileName, Channel channel);
    const std::vector<EQNode> getEQNodes (juce::String profileName, Channel channel) const;
    
    // Profiles
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEQValueTree>> getProfileNamed (juce::String profileName) const;
    
    // Setting points
    int addEQNode (float frequency, float amplitude, float pan, juce::String profileName, Channel channel);
    void removeEQNode (int id, juce::String profileName, Channel channel);
    void updateEQNode (int id, float frequency, float amplitude, float pan, juce::String profileName, Channel channel);
    void clearEQNodes (juce::String profileName);
    
    // Changing points
    void startCalibratingEQNode (EQNode eqNode, Channel channel);
    void updateCalibratingEQNode (EQNode eqNode, Channel channel);
    void endCalibratingEQNode();
    float getCurrPlayingFreq();
    
    // Testing
    void startTestingAt (float freq, juce::String profileName, Channel channel);
    void updateTestingAt (float freq, juce::String profileName, Channel channel);
    void endTesting();
    float getCurrTestingFreq();
    
    // Sine sweep
    void startSineSweep (float centerFreq, std::optional<float> ampl = std::nullopt);
    void updateSineSweep (float centerFreq, std::optional<float> ampl = std::nullopt);
    void endSineSweep();
    float getCurrSineSweepFreq();
    
    // misc
    void setReferenceVolume (float volume);
    
    // Listener
    void addListener (Listener* listener);
    void removeListener();
    
    // Reference values
    void setReferenceFreq1 (float freq);
    void setReferenceFreq2 (float freq);
    void setReferenceAmplLeft1 (float ampl);
    void setReferenceAmplRight1 (float ampl);
    void setReferenceAmplLeft2 (float ampl);
    void setReferenceAmplRight2 (float ampl);
    void setReferenceCrossfeedGainLeft1 (float gain);
    void setReferenceCrossfeedGainRight1 (float gain);
    void setReferenceCrossfeedGainLeft2 (float gain);
    void setReferenceCrossfeedGainRight2 (float gain);
    
    void setCrossfeedDelayInMs (float delayInMs);

private:
    std::optional<std::reference_wrapper<CabinEQValueTree>> profileNamed (juce::String profileName) const; // returns the current profile. Crashes if currentProfileId doesn't match an existing profile.
    
    static const int FFT_SIZE = 10;

    PlaybackManager playbackManager;
    CabinEQValueTreeManager cabinEQValueTreeManager;
    
    juce::dsp::ProcessSpec spec;
    
    Listener* listener = nullptr;
    bool hasLoadedData = false;
    juce::String currProfileName { "" };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartupMVPAudioProcessor)
};
