/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CabinEqAudioProcessor.h"
#include "CabinEqProcessorEditor.h"
#include "CabinStandaloneFilterWindow.h"

namespace
{
    const juce::Identifier idEditorWidth { "editorWidth" };
    const juce::Identifier idEditorHeight { "editorHeight" };
}

//==============================================================================
CabinEqAudioProcessor::CabinEqAudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       parameters (*this, nullptr, "Params", createParameterLayout()),
       profiles (parameters, undoManager)
{
    bypassParameter = dynamic_cast<juce::AudioParameterBool*> (parameters.getParameter (ParamIDs::bypass));
    autoGainParameter = parameters.getRawParameterValue (ParamIDs::autoGain);
    volumeParameter = parameters.getRawParameterValue (ParamIDs::volume);
    crossfeedParameter = parameters.getRawParameterValue (ParamIDs::crossfeed);
    crossfeedLevelParameter = parameters.getRawParameterValue (ParamIDs::crossfeedLevel);
    crossfeedDelayParameter = parameters.getRawParameterValue (ParamIDs::crossfeedDelay);

    profiles.ensureValidState();
    currentSelection = profiles.getSelectedProfileName();
    parameters.state.addListener (this);
    refresh();
    updateStateSnapshot();

    startTimer (250); // keeps the state snapshot fresh, and saves the standalone app's state
}

CabinEqAudioProcessor::~CabinEqAudioProcessor()
{
    stopTimer();
    cancelPendingUpdate();
    parameters.state.removeListener (this);
}

//==============================================================================
const juce::String CabinEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CabinEqAudioProcessor::acceptsMidi() const   { return false; }
bool CabinEqAudioProcessor::producesMidi() const  { return false; }
bool CabinEqAudioProcessor::isMidiEffect() const  { return false; }

double CabinEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CabinEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CabinEqAudioProcessor::getCurrentProgram()                           { return 0; }
void CabinEqAudioProcessor::setCurrentProgram (int)                      {}
const juce::String CabinEqAudioProcessor::getProgramName (int)           { return {}; }
void CabinEqAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void CabinEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) std::max (getTotalNumInputChannels(), getTotalNumOutputChannels());

    currentSampleRate = sampleRate;
    refresh(); // the curve, and so auto gain, depend on the sample rate
    playbackManager.prepare (spec);
}

void CabinEqAudioProcessor::releaseResources()
{
}

bool CabinEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& output = layouts.getMainOutputChannelSet();
    if (output != juce::AudioChannelSet::mono() && output != juce::AudioChannelSet::stereo())
        return false;
    return output == layouts.getMainInputChannelSet();
}

void CabinEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto& crossfeed = playbackManager.getCrossfeed();
    crossfeed.setEnabled (crossfeedParameter->load() >= 0.5f);
    crossfeed.setLevelDb (crossfeedLevelParameter->load());
    crossfeed.setDelayMs (crossfeedDelayParameter->load());

    const bool autoGainOn = autoGainParameter->load() >= 0.5f;
    playbackManager.setGainDb (preampDb.load() + (autoGainOn ? autoGainDb.load() : 0.0f));
    playbackManager.setBypassed (bypassParameter->get());
    playbackManager.setVolumeDb (volumeParameter->load());

    playbackManager.processBlock (buffer);
}

//==============================================================================
bool CabinEqAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CabinEqAudioProcessor::createEditor()
{
    return new CabinEqProcessorEditor (*this);
}

//==============================================================================
void CabinEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // The UI edits the state on the message thread without locking it, so other threads get a copy
    if (! juce::MessageManager::existsAndIsCurrentThread())
    {
        const juce::ScopedLock lock (snapshotLock);
        destData = stateSnapshot;
        return;
    }

    auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void CabinEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (parameters.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml (*xml);
    CabinEqProfileManager::migrateState (state);

    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        applyState (state);
        return;
    }

    // Off the message thread, the audio gets the new bands now (hosts rendering offline might never
    // run the message loop), and the state itself is swapped in on the message thread
    pushBandsToAudio (CabinEqProfileManager::getSelectedBandProfile (state));
    {
        const juce::ScopedLock lock (snapshotLock);
        if (auto stateXml = state.createXml())
            copyXmlToBinary (*stateXml, stateSnapshot);
    }

    juce::MessageManager::callAsync ([safeThis = juce::WeakReference<CabinEqAudioProcessor> (this), state]
    {
        if (safeThis != nullptr)
            safeThis->applyState (state);
    });
}

void CabinEqAudioProcessor::applyState (const juce::ValueTree& state)
{
    {
        const juce::ScopedLock lock (refreshLock);
        parameters.replaceState (state);
        profiles.ensureValidState();
        currentSelection = previousSelection = profiles.getSelectedProfileName();
    }

    undoManager.clearUndoHistory();
    refresh();
    updateStateSnapshot();
}

void CabinEqAudioProcessor::updateStateSnapshot()
{
    juce::MemoryBlock block;
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, block);

    const juce::ScopedLock lock (snapshotLock);
    stateSnapshot = std::move (block);
    snapshotIsStale = false;
}

juce::AudioProcessorParameter* CabinEqAudioProcessor::getBypassParameter() const
{
    return bypassParameter;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CabinEqAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CabinEqAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterBool> (ParameterID (ParamIDs::bypass, 1), "Bypass", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID (ParamIDs::autoGain, 1), "Auto Gain", true));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID (ParamIDs::volume, 1), "Volume",
                                                       NormalisableRange<float> (-30.0f, 24.0f, 0.1f), 0.0f,
                                                       AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID (ParamIDs::crossfeed, 1), "Crossfeed", false));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID (ParamIDs::crossfeedLevel, 1), "Crossfeed Level",
                                                       NormalisableRange<float> (-24.0f, -3.0f, 0.1f), -9.0f,
                                                       AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID (ParamIDs::crossfeedDelay, 1), "Crossfeed Delay",
                                                       NormalisableRange<float> (0.0f, CrossfeedProcessor::maxDelayMs, 0.01f), 0.3f,
                                                       AudioParameterFloatAttributes().withLabel ("ms")));
    return layout;
}

//==============================================================================
void CabinEqAudioProcessor::selectProfile (const juce::String& profileName)
{
    if (profiles.getProfileNamed (profileName).has_value())
        profiles.setSelectedProfileName (profileName);
}

void CabinEqAudioProcessor::undo()
{
    isUndoingOrRedoing = true;
    profileChangedByUndo = {};
    undoManager.undo();
    isUndoingOrRedoing = false;

    if (profileChangedByUndo.isNotEmpty())
        selectProfile (profileChangedByUndo);
}

void CabinEqAudioProcessor::redo()
{
    isUndoingOrRedoing = true;
    profileChangedByUndo = {};
    undoManager.redo();
    isUndoingOrRedoing = false;

    if (profileChangedByUndo.isNotEmpty())
        selectProfile (profileChangedByUndo);
}

bool CabinEqAudioProcessor::isAutoGainOn() const
{
    return autoGainParameter->load() >= 0.5f;
}

double CabinEqAudioProcessor::getCurveSampleRate() const
{
    const double sampleRate = currentSampleRate.load();
    return sampleRate > 0.0 ? sampleRate : 48000.0;
}

void CabinEqAudioProcessor::showAudioSettingsDialog()
{
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
        holder->showAudioSettingsDialog();
}

juce::Point<int> CabinEqAudioProcessor::getEditorSize() const
{
    return { (int) parameters.state.getProperty (idEditorWidth, 1080),
             (int) parameters.state.getProperty (idEditorHeight, 680) };
}

void CabinEqAudioProcessor::setEditorSize (juce::Point<int> size)
{
    parameters.state.setProperty (idEditorWidth, size.x, nullptr);
    parameters.state.setProperty (idEditorHeight, size.y, nullptr);
}

//==============================================================================
void CabinEqAudioProcessor::refresh()
{
    const juce::ScopedLock lock (refreshLock);

    auto selected = profiles.getSelectedProfile();
    if (! selected.isValid())
        return;

    auto bandProfile = selected.getBandProfile();
    pushBandsToAudio (bandProfile);
    if (bandProfile.isCurve())
    {
        // Split ears: as loud as the two on average
        autoGainDb = -CurveResponse (bandProfile.getPoints (0)).loudnessChangeDb();
        if (bandProfile.isSplit())
            autoGainDb = 0.5f * (autoGainDb - CurveResponse (bandProfile.getPoints (1)).loudnessChangeDb());
    }
    else
    {
        curve.setSampleRate (getCurveSampleRate());
        curve.updateWithBands (bandProfile.getBands());
        autoGainDb = -curve.loudnessChangeDb();
    }
}

void CabinEqAudioProcessor::pushBandsToAudio (const BandProfile& bandProfile)
{
    const juce::ScopedLock lock (refreshLock);

    // Only one kind plays: the other fades out
    if (bandProfile.isCurve())
    {
        playbackManager.setBands ({});
        playbackManager.setCurve (bandProfile.getPoints (0),
                                  bandProfile.isSplit() ? std::optional (bandProfile.getPoints (1)) : std::nullopt);
    }
    else
    {
        playbackManager.setBands (bandProfile.getBands());
        playbackManager.setCurve (std::nullopt);
    }
    preampDb = bandProfile.getVolume();
}

void CabinEqAudioProcessor::handleAsyncUpdate()
{
    // Undo can take away the selected profile, or the last one. Go back to the one you had before.
    profiles.ensureValidState (previousSelection);
    refresh();
    updateStateSnapshot();
    stateChanged.sendSynchronousChangeMessage();
}

void CabinEqAudioProcessor::timerCallback()
{
    if (snapshotIsStale)
        updateStateSnapshot();

    const auto now = juce::Time::getMillisecondCounter();
    if (needsSaving && isStandalone() && now - lastSaveTime > 2000)
    {
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
            holder->savePluginState();
        needsSaving = false;
        lastSaveTime = now;
    }
}

void CabinEqAudioProcessor::treeChanged (const juce::ValueTree& changedTree)
{
    if (changedTree.hasType ("PARAM"))
    {
        snapshotIsStale = true; // parameters get to the audio thread on their own
        return;
    }

    if (isUndoingOrRedoing)
    {
        auto name = profiles.getProfileNameContaining (changedTree);
        if (name.isNotEmpty())
            profileChangedByUndo = name;
    }

    needsSaving = true;

    // Update the filters right away when it's safe, so dragging a band feels immediate.
    // Auto gain, which takes more working out, catches up once the changes settle.
    // Mid-rename or mid-undo there may briefly be no selected profile; don't send silence.
    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        auto selected = profiles.getSelectedProfile();
        if (selected.isValid())
            pushBandsToAudio (selected.getBandProfile());
    }
    triggerAsyncUpdate();
}

void CabinEqAudioProcessor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == parameters.state && property == CabinEqProfileManager::idSelectedProfile)
    {
        auto newSelection = profiles.getSelectedProfileName();
        if (newSelection != currentSelection)
        {
            previousSelection = currentSelection;
            currentSelection = newSelection;
        }
    }
    treeChanged (tree);
}

void CabinEqAudioProcessor::valueTreeChildAdded (juce::ValueTree&, juce::ValueTree& child)
{
    treeChanged (child);
}

void CabinEqAudioProcessor::valueTreeChildRemoved (juce::ValueTree& parent, juce::ValueTree& child, int)
{
    // The child is detached now, so look at where it was
    if (isUndoingOrRedoing && child.hasType (CabinEqProfile::idProfile))
        profileChangedByUndo = {}; // a profile was removed; there's nothing of it to show
    treeChanged (child.hasType (CabinEqProfile::idProfile) ? juce::ValueTree() : parent);
}

void CabinEqAudioProcessor::valueTreeChildOrderChanged (juce::ValueTree& parent, int, int)
{
    treeChanged (parent);
}

void CabinEqAudioProcessor::valueTreeRedirected (juce::ValueTree&)
{
    needsSaving = true;
    triggerAsyncUpdate();
}
