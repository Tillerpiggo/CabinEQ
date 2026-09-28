/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PlaybackManager.h"
#include "CabinEqProfileManager.h"
#include "BandEqCurve.h"

namespace ParamIDs
{
    inline const juce::String bypass { "bypass" };
    inline const juce::String volume { "volume" };
    inline const juce::String autoGain { "autoGain" };
    inline const juce::String crossfeed { "crossfeed" };
    inline const juce::String crossfeedLevel { "crossfeedLevel" };
    inline const juce::String crossfeedDelay { "crossfeedDelay" };
}

//==============================================================================
/// A parametric EQ with named profiles. The profiles live in the parameters' ValueTree,
/// which is the one source of truth: anything that edits it (the UI, undo, loading state)
/// ends up in refresh(), which hands the selected profile to the audio path.
class CabinEqAudioProcessor  : public juce::AudioProcessor,
                               private juce::ValueTree::Listener,
                               private juce::AsyncUpdater,
                               private juce::Timer
{
public:
    //==============================================================================
    CabinEqAudioProcessor();
    ~CabinEqAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
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

    juce::AudioProcessorParameter* getBypassParameter() const override;

    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

    // Message thread only, from here down
    CabinEqProfileManager& getProfiles() { return profiles; }
    CabinEqProfile getSelectedProfile() const { return profiles.getSelectedProfile(); }
    BandProfile getSelectedBandProfile() const { return profiles.getSelectedProfile().getBandProfile(); }
    void selectProfile (const juce::String& profileName);

    juce::UndoManager& getUndoManager() { return undoManager; }
    /// Undo and redo also select the profile they changed, so you can see what happened.
    void undo();
    void redo();

    /// Auto gain's current correction in dB, whether or not it's switched on.
    float getAutoGainDb() const { return autoGainDb; }
    bool isAutoGainOn() const;
    double getCurveSampleRate() const;

    SpectrumAnalyzer& getAnalyzer() { return playbackManager.getAnalyzer(); }
    CalibrationPlayer& getCalibration() { return playbackManager.getCalibration(); }
    bool isStandalone() const { return wrapperType == wrapperType_Standalone; }
    void showAudioSettingsDialog();

    /// Tells the UI that the profiles changed. Always on the message thread.
    juce::ChangeBroadcaster stateChanged;

    // Editor size, remembered in the state
    juce::Point<int> getEditorSize() const;
    void setEditorSize (juce::Point<int> size);

private:
    void refresh(); // pushes the selected profile to the audio path, and works out auto gain
    void pushBandsToAudio (const BandProfile& bandProfile);
    void applyState (const juce::ValueTree& state); // message thread
    void updateStateSnapshot();
    void handleAsyncUpdate() override;
    void timerCallback() override;

    void valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property) override;
    void valueTreeChildAdded (juce::ValueTree& parent, juce::ValueTree& child) override;
    void valueTreeChildRemoved (juce::ValueTree& parent, juce::ValueTree& child, int index) override;
    void valueTreeChildOrderChanged (juce::ValueTree& parent, int oldIndex, int newIndex) override;
    void valueTreeRedirected (juce::ValueTree& tree) override;
    void treeChanged (const juce::ValueTree& changedTree);

    juce::UndoManager undoManager;
    CabinEqProfileManager profiles;
    PlaybackManager playbackManager;
    BandEqCurve curve;

    juce::CriticalSection refreshLock; // setStateInformation can come from any thread
    std::atomic<float> preampDb { 0.0f };
    std::atomic<float> autoGainDb { 0.0f };
    std::atomic<double> currentSampleRate { 0.0 };

    juce::AudioParameterBool* bypassParameter = nullptr;
    std::atomic<float>* autoGainParameter = nullptr;
    std::atomic<float>* volumeParameter = nullptr;
    std::atomic<float>* crossfeedParameter = nullptr;
    std::atomic<float>* crossfeedLevelParameter = nullptr;
    std::atomic<float>* crossfeedDelayParameter = nullptr;

    bool isUndoingOrRedoing = false;
    juce::String profileChangedByUndo;
    juce::String currentSelection, previousSelection;
    bool needsSaving = false;
    juce::uint32 lastSaveTime = 0;

    // For hosts that ask for the state from other threads
    juce::CriticalSection snapshotLock;
    juce::MemoryBlock stateSnapshot;
    bool snapshotIsStale = false;

    JUCE_DECLARE_WEAK_REFERENCEABLE (CabinEqAudioProcessor)

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CabinEqAudioProcessor)
};
