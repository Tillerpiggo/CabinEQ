/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PlaybackManager.h"
#include "CabinEqProfileManager.h"
#include "GlyphManager.h"
#include "CabinPeqGraph.h"
#include "Listeners.h"

//==============================================================================
/**
*/
class CabinEqAudioProcessor  : public juce::AudioProcessor,
                               public CabinPeqGraphListener,
                               public CabinPeqGraphDataSource,
                               public NoiseGridViewListener,
                               public NoiseGridViewDataSource
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
    
    void setVolume (float volume);
    void setIsFilterOn (bool isFilterOn);
    void setIsPlaying (bool isPlaying);
    void setSpeedFactor (float speedFactor);
    void setBandwidth (float bandwidth);
    
    // Provisional bands
    void setProvisionalBands (std::vector<Band> provisionalBands);
    void setProvisionalBandsOn (bool provisionalBandsOn);
    
    // Profiles
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void renameProfile (juce::String profileName, juce::String newProfileName);
    void setProfileVolume (float masterVolume) override;
    const std::vector<juce::String> getProfileNames() const;
    std::optional<std::reference_wrapper<CabinEqProfile>> getProfileNamed (juce::String profileName) const;
    BandProfile getBandProfile() override;
    
    std::optional<juce::String> getLastSelectedProfileName();
    void setLastSelectedProfileName (juce::String profileName);
    
    // Setting bands
    void updateFilter(); // updates the filter to match whatever bands are associated with profileName
    int addBand (float freq, float ampl, float bandwidth, Band::Type type) override;
    void updateBand (int id, float freq, float ampl, float bandwidth, Band::Type type) override;
    void removeBand (int id) override;
    
    // NoiseGridViewListener + NoiseGridViewDataSource
    void addSequence (NoiseSequence sequence) override;
    void addSequenceWithCoords (std::vector<std::pair<int, int>> coords) override;
    void removeSequence (std::pair<int, int> origin) override;
    void moveSequence (std::pair<int, int> origin, std::pair<int, int> newOrigin) override;
    void toggleCoords (std::pair<int, int> point) override;
    
    NoiseSequenceGrid getNoiseGrid() override;
    std::pair<int, int> getNumRowsAndNumCols() override;
    
    // Listener
    void addListener (Listener* listener);
    void removeListener();
    
    // Glyph methods
    bool hasNextGlyph();
    bool hasPrevGlyph();
    void goToNextGlyph();
    void goToPrevGlyph();
    Glyph getCurrGlyph();
    
    void setSizeFactor (float sizeFactor);
    void setCenterPos (juce::Point<float> centerPos);
    float getSizeFactor() const;
    juce::Point<float> getCenterPos() const;
    
    float getCurrPlayingTime();
    

private:
    std::optional<std::reference_wrapper<CabinEqProfile>> profileNamed (juce::String profileName) const; // returns the current profile. Crashes if currentProfileId doesn't match an existing profile.

    PlaybackManager playbackManager;
    CabinEqProfileManager cabinEqProfileManager;
    GlyphManager glyphManager;
    NoiseSequenceGrid noiseSequenceGrid { 3, 3 };
    
    juce::dsp::ProcessSpec spec;
    
    std::vector<Listener*> listeners;
    bool hasLoadedData = false;
    juce::String currProfileName { "" };
    juce::String profileId { "NO_PROFILE" };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqAudioProcessor)
};
