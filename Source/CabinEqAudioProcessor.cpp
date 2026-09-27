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
    crossfeedParameter = parameters.getRawParameterValue (ParamIDs::crossfeed);
    crossfeedLevelParameter = parameters.getRawParameterValue (ParamIDs::crossfeedLevel);
    crossfeedDelayParameter = parameters.getRawParameterValue (ParamIDs::crossfeedDelay);

    profiles.ensureValidState();
    parameters.state.addListener (this);
    refresh();

    startTimer (2000); // the standalone app saves its state when it changes
}

CabinEqAudioProcessor::~CabinEqAudioProcessor()
{
    parameters.state.removeListener (this);
    cancelPendingUpdate();
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

    {
        const juce::ScopedLock lock (refreshLock);
        parameters.replaceState (state);
        profiles.ensureValidState();
    }

    undoManager.clearUndoHistory();

    // Hosts that render offline might never run the message loop, so update the audio path now
    refresh();
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

    auto bandProfile = pushBandsToAudio();
    curve.setSampleRate (getCurveSampleRate());
    curve.updateWithBands (bandProfile.getBands());
    autoGainDb = -curve.loudnessChangeDb();
}

BandProfile CabinEqAudioProcessor::pushBandsToAudio()
{
    const juce::ScopedLock lock (refreshLock);

    auto bandProfile = profiles.getSelectedProfile().getBandProfile();
    playbackManager.setBands (bandProfile.getBands());
    preampDb = bandProfile.getVolume();
    return bandProfile;
}

void CabinEqAudioProcessor::handleAsyncUpdate()
{
    profiles.ensureValidState(); // undo can take away the selected profile, or the last one
    refresh();
    stateChanged.sendSynchronousChangeMessage();
}

void CabinEqAudioProcessor::timerCallback()
{
    if (needsSaving && isStandalone())
    {
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
            holder->savePluginState();
        needsSaving = false;
    }
}

void CabinEqAudioProcessor::treeChanged (const juce::ValueTree& changedTree)
{
    if (changedTree.hasType ("PARAM"))
        return; // parameters get to the audio thread on their own

    if (isUndoingOrRedoing)
    {
        auto name = profiles.getProfileNameContaining (changedTree);
        if (name.isNotEmpty())
            profileChangedByUndo = name;
    }

    needsSaving = true;

    // Update the filters right away when it's safe, so dragging a band feels immediate.
    // Auto gain, which takes more working out, catches up once the changes settle.
    if (juce::MessageManager::existsAndIsCurrentThread())
        pushBandsToAudio();
    triggerAsyncUpdate();
}

void CabinEqAudioProcessor::valueTreePropertyChanged (juce::ValueTree& tree, const juce::Identifier&)
{
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
