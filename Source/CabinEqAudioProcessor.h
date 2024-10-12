/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Curve.h"
#include "PlaybackManager.h"
#include "CabinEqValueTreeManager.h"

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
    
    void applyCurve (int fftSize, juce::String profileName);
    void setIsProcessing (bool isProcessing);
    void setBypassBalance (float balance);
    void setVolume (float volume);
    void setMutedGens (std::vector<bool> mutedGens);
    
    std::optional<std::reference_wrapper<Curve>> getAmplCurve (juce::String profileName);
    std::optional<std::reference_wrapper<Curve>> getPanCurve (juce::String profileName);
    std::optional<std::reference_wrapper<Curve>> getPhaseCurve (juce::String profileName);
    const std::vector<CurvePt> getAmplPts (juce::String profileName) const;
    const std::vector<CurvePt> getPanPts (juce::String profileName) const;
    const std::optional<CurvePt> getAmplPtWithId (int id, juce::String profileName) const;
    const std::optional<CurvePt> getPanPtWithId (int id, juce::String profileName) const;
    
    // Profiles
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void renameProfile (juce::String profileName, juce::String newProfileName);
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEqValueTree>> getProfileNamed (juce::String profileName) const;
    
    std::optional<juce::String> getLastSelectedProfileName();
    void setLastSelectedProfileName (juce::String profileName);
    
    // Setting points
//    int addEQNode (float frequency, float amplitude, float pan, juce::String profileName);
//    void removeEQNode (int id, juce::String profileName);
//    void updateEQNode (int id, float frequency, float amplitude, float pan, juce::String profileName);
//    void clearEQNodes (juce::String profileName);
    
    // Setting points (new)
    int addAmplPt (const float freq, const float ampl, juce::String profileName);
    int addPanPt (const float freq, const float pan, juce::String profileName);
    int addPhasePt (const float freq, const float pan, juce::String profileName);
    void removeAmplPt (const int id, juce::String profileName);
    void removePanPt (const int id, juce::String profileName);
    void removePhasePt (const int id, juce::String profileName);
    void updateAmplPt (const int id, const float freq, const float ampl, juce::String profileName);
    void updatePanPt (const int id, const float freq, const float pan, juce::String profileName);
    void updatePhasePt (const int id, const float freq, const float phase, juce::String profileName);
    void clearEQNodes (juce::String profileName);
    
    // Changing points
//    void startPlayingFreq (float freq, juce::String profileName);
//    void updatePlayingFreq (float freq, juce::String profileName);
//    void endCalibratingEQNode();
//    float getCurrPlayingFreq();
    
    void startAmplCalibration (float freq, juce::String profileName);
    void updateAmplCalibration (float freq, juce::String profileName);
    void startPanCalibration (float freq, juce::String profileName);
    void updatePanCalibration (float freq, juce::String profileName);
    void startPhaseCalibration (float freq, juce::String profileName);
    void updatePhaseCalibration (float freq, juce::String profileName);
    void stopCalibration();
    float getCurrPlayingFreq();
    
    // Testing
    void setPatternSolo (bool solo);
    
    // Sine sweep
    void startSineSweep (float centerFreq, juce::String profileName);
    void updateSineSweep (float centerFreq, juce::String profileName);
    void endSineSweep();
    float getCurrSineSweepFreq();
    
    // misc
    void setReferenceVolume (float volume);
    void setReferencePan (float pan);
    void setReferenceVolume1 (float volume);
    void setReferenceVolume2 (float volume);
    
    // Listener
    void addListener (Listener* listener);
    void removeListener();

private:
    std::optional<std::reference_wrapper<CabinEqValueTree>> profileNamed (juce::String profileName) const; // returns the current profile. Crashes if currentProfileId doesn't match an existing profile.
    
    static const int FFT_SIZE = 10;

    PlaybackManager playbackManager;
    CabinEqValueTreeManager cabinEqValueTreeManager;
    
    juce::dsp::ProcessSpec spec;
    
    std::vector<Listener*> listeners;
    bool hasLoadedData = false;
    juce::String currProfileName { "" };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqAudioProcessor)
};
