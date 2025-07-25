/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PlaybackManager.h"
#include "CabinEqProfileManager.h"
#include "CheckerboardManager.h"
#include "CabinPeqGraph.h"
#include "Listeners.h"
#include "FreeTrialBanner.h"
#include "CabinEqMarketplaceStatus.h"
#include "CabinStandaloneFilterWindow.h"
#include "GlyphManager.h"

//==============================================================================
/**
*/
class CabinEqAudioProcessor  : public juce::AudioProcessor,
                               public AudioPlayerComponentListener,
                               public CabinPeqGraphListener,
                               public CabinPeqGraphDataSource,
                               public CheckerboardViewListener,
                               public CheckerboardViewDataSource,
                               public CalibrationListener,
                               public ContactUsBannerListener,
                               public GlyphViewListener,
                               public GlyphViewDataSource,
                               public ProfileViewDataSource,
                               public juce::Timer
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
    
    void freeTrialDidReset();
    
    // AudioPlayerComponentListener
    void setIsAudioFilePlaying (bool isPlaying) override;
    void setFile (juce::File file) override;
    void addAsListener (PlaybackManagerListener* listener) override;
    
    // Calibration listener methods
    void setVolume (float volume) override;
    void setCalibrationVolume (float calibrationVolume) override;
    void setIsFilterOn (bool isFilterOn) override;
    void setIsPlaying (bool isPlaying) override;
    void setIsCabinNoise (bool isCabinNoise) override;
    void setMinFreq (float newMinFreq) override;
    void setIIR (bool isIIR) override;
    void updateFIRFilter() override;
    void setFIRQuality (int fftSize) override;
    void setPinkNoise (bool pinkNoiseEnabled) override;
    void setSpeedFactor (float speedFactor) override;
    void setBandwidth (float bandwidth) override;
    void setGlyphVolume(float volume) override;
    
    void setCrossfeedDelaySamples (int samples) override;
    void setCrossfeedVolume (float volume) override;
    void setCrossfeedEnabled (bool enabled) override;
    
    void goToNext() override;
    void goToPrev() override;
    bool hasNext() override;
    bool hasPrev() override;
    void toggleCheckerboardPolarity() override;
    void selectCheckerboardAtIdx (int idx) override;
    void setSoloSquareCoords (std::set<std::pair<int, int>> soloSquareCoords) override;
    
    // Provisional bands
    void setProvisionalBands (std::vector<Band> provisionalBands);
    void setProvisionalBandsOn (bool provisionalBandsOn);
    
    // Profiles
    void addProfile (juce::String profileName);
    void addDuplicateProfile (juce::String profileName, juce::String oldProfileName);
    void removeProfile (juce::String profileName);
    void renameProfile (juce::String profileName, juce::String newProfileName);
    void setProfileVolume (float masterVolume) override;
    bool isProfileLocked();
    std::optional<std::reference_wrapper<CabinEqProfile>> getProfileNamed (juce::String profileName) const;
    BandProfile getBandProfile() override;
    
    std::optional<juce::String> getLastSelectedProfileName();
    float getMasterVolume();
    bool getHasLicense();
    void setLastSelectedProfileName (juce::String profileName);
    void setMasterVolume (float masterVolume);
    void setHasLicense (bool hasLicense);
    
    // Setting bands
    void updateFilter(); // updates the filter to match whatever bands are associated with profileName
    int addMultiBandStep() override;
    void removeMultiBandStep (int stepId) override;
    void setStepEnabled (int stepId, bool isEnabled) override;
    int addBand (float freq, float ampl, float bandwidth, Band::Type type, Band::FilterType filterType, int stepId) override;
    void updateBand (int bandId, float freq, float ampl, float bandwidth, Band::Type type, int stepId) override;
    void removeBand (int bandId, int stepId) override;
    
    
    std::vector<float> getCurrPlayingFreqs() override;
    std::vector<std::pair<float, float>> getCurrPlayingFreqsAndVols() override;
    
    float getBandwidth() override;
    
    // CheckerboardViewDataSource
    const Checkerboard getCheckerboard() override;
    bool getIsPlaying() override;
    int getNumCheckerboards() override;
    std::string getNameAtIdx (int idx) override;
    int getSelectedRow() override;

    // GlyphViewListener
    void addGlyph (ArchetypalGlyph archetype, juce::Point<float> centerPos, float sizeFactor) override;
    void moveGlyph (int glyphId, juce::Point<float> centerPos) override;
    void removeGlyph (int glyphId) override;
    void incrementGlyphVolume (int glyphId, float increment) override;
    void incrementSizeFactor (int glyphId, float horizontalIncrement, float verticalIncrement) override;
    void moveGlyphs (std::unordered_map<int, juce::Point<float>> idsToPositions) override;
    void scaleGlyphs (std::unordered_set<int> glyphIds, float increment) override;

    // GlyphViewDataSource
    const std::vector<ArchetypalGlyph>& getArchetypalGlyphs() override;
    const std::vector<Glyph>& getGlyphs() override;
    float getCurrPlayingTime() override;

    // ProfileViewListener + ProfileViewDataSource
    // void addProfile() override;
    // void duplicateProfile (int rowIdx) override;
    // void renameProfile (int rowIdx) override;
    // void deleteProfile (int rowIdx) override;
    std::vector<juce::String> getProfileNames() override;
    bool getIsProfileLocked (int rowIdx) override;
    
    // StandalonePlugin/AudioDeviceManager methods
    void saveData();
    void restartAudio() override;
    void showAudioSettingsDialog();
    
    CabinEqMarketplaceStatus& getMarketplaceStatus();
    
    // Listener
    void addListener (Listener* listener);
    void removeListener();
    
    // Timer
    void timerCallback() override; // used to autosave

private:
    std::optional<std::reference_wrapper<CabinEqProfile>> profileNamed (juce::String profileName) const; // returns the current profile. Crashes if currentProfileId doesn't match an existing profile.

    PlaybackManager playbackManager;
    CabinEqProfileManager cabinEqProfileManager;
    CheckerboardManager checkerboardManager;
    GlyphManager glyphManager;
    
    bool dataHasChanged = false;
    
    CabinEqMarketplaceStatus marketplaceStatus;
    
    juce::dsp::ProcessSpec spec;
    
    std::vector<Listener*> listeners;
    bool hasLoadedData = false;
    juce::String currProfileName { "" };
    juce::String profileId { "NO_PROFILE" };
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqAudioProcessor)
};
