/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <chrono>

//==============================================================================
StartupMVPAudioProcessor::StartupMVPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          cabinEQValueTreeManager (parameters)

#endif
{
    
}

StartupMVPAudioProcessor::~StartupMVPAudioProcessor()
{
}

//==============================================================================
const juce::String StartupMVPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StartupMVPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StartupMVPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StartupMVPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StartupMVPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StartupMVPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StartupMVPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String StartupMVPAudioProcessor::getProgramName (int index)
{
    return {};
}

void StartupMVPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void StartupMVPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
}

void StartupMVPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StartupMVPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StartupMVPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear buffer before handing it off to playbackManager
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    playbackManager.processBlock (buffer);
}

//==============================================================================
bool StartupMVPAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StartupMVPAudioProcessor::createEditor()
{
    return new StartupMVPAudioProcessorEditor (*this);
}

//==============================================================================
void StartupMVPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StartupMVPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get())
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            cabinEQValueTreeManager.initProfiles();
            
            if (! hasLoadedData)
            {
                for (auto listener : listeners)
                    if (listener != nullptr)
                        listener->didLoadData();
                hasLoadedData = true;
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StartupMVPAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout StartupMVPAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void StartupMVPAudioProcessor::applyCurve (int fftSize, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateFilterWithCurves (profile->get().getAmplCurve(), profile->get().getPanCurve(), fftSize);
    }
        
}

void StartupMVPAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
}

void StartupMVPAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

void StartupMVPAudioProcessor::setWetVolume (float wetVolume)
{
    playbackManager.setWetVolume (wetVolume);
}

void StartupMVPAudioProcessor::setDryVolume (float dryVolume)
{
    playbackManager.setDryVolume (dryVolume);
}

std::optional<std::reference_wrapper<Curve>> StartupMVPAudioProcessor::getAmplCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplCurve();
    std::cout << "unable to get ampl curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

std::optional<std::reference_wrapper<Curve>> StartupMVPAudioProcessor::getPanCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanCurve();
    std::cout << "unable to get pan curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

const std::vector<CurvePt> StartupMVPAudioProcessor::getAmplPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplPts();
    return {};
}

const std::vector<CurvePt> StartupMVPAudioProcessor::getPanPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanPts();
    return {};
}

const std::optional<CurvePt> StartupMVPAudioProcessor::getAmplPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplPtWithId (id);
    return std::nullopt;
}

const std::optional<CurvePt> StartupMVPAudioProcessor::getPanPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanPtWithId (id);
    return std::nullopt;
}

int StartupMVPAudioProcessor::addAmplPt (const float freq, const float ampl, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addAmplPt (freq, ampl);
    return -1;
}

int StartupMVPAudioProcessor::addPanPt (const float freq, const float pan, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addPanPt (freq, pan);
    return -1;
}

void StartupMVPAudioProcessor::removeAmplPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeAmplPt (id);
}

void StartupMVPAudioProcessor::removePanPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removePanPt (id);
}

void StartupMVPAudioProcessor::updateAmplPt (const int id, const float freq, const float ampl, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateAmplPt (id, freq, ampl);
}

void StartupMVPAudioProcessor::updatePanPt (const int id, const float freq, const float pan, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updatePanPt (id, freq, pan);
}

void StartupMVPAudioProcessor::clearEQNodes (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().resetNodes();
}

void StartupMVPAudioProcessor::startPlayingFreq (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.startPlayingFreq (freq,
                                          profile->get().getAmplCurve(),
                                          profile->get().getPanCurve());
        playbackManager.setIsCalibrating (true);
    }
}

// TODO: Hacky for testing, will remove later
void StartupMVPAudioProcessor::startPlayingReferenceFreqs()
{
    playbackManager.setIsCalibrating (true);
    playbackManager.startPlayingReferenceFreqs();
}

void StartupMVPAudioProcessor::updatePlayingReferenceFreqs()
{
    playbackManager.updatePlayingReferenceFreqs();
}

void StartupMVPAudioProcessor::updatePlayingFreq (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updatePlayingFreq (freq,
                                          profile->get().getAmplCurve(),
                                          profile->get().getPanCurve());
    }
}

void StartupMVPAudioProcessor::endCalibratingEQNode()
{
    playbackManager.setIsCalibrating (false);
}

float StartupMVPAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
}

void StartupMVPAudioProcessor::startTestingAt (float freq, juce::String profileName)
{
    auto curve = getAmplCurve (profileName); // hacky for now
    if (curve.has_value())
        playbackManager.startTestingFreq (freq, curve->get());
}

void StartupMVPAudioProcessor::updateTestingAt (float freq, juce::String profileName)
{
    auto curve = getAmplCurve (profileName); // hacky for now
    if (curve.has_value())
        playbackManager.updateTestingFreq (freq, curve->get());
}

void StartupMVPAudioProcessor::endTesting()
{
    playbackManager.stopTestingFreq();
}

float StartupMVPAudioProcessor::getCurrTestingFreq()
{
    return playbackManager.getCurrTestingFreq();
}

void StartupMVPAudioProcessor::addProfile (juce::String profileName)
{
    cabinEQValueTreeManager.addProfile (profileName);
}

void StartupMVPAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEQValueTreeManager.addDuplicateProfile (profileName, oldProfileName);
}

void StartupMVPAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEQValueTreeManager.removeProfile (profileName);
}

const std::vector<juce::String> StartupMVPAudioProcessor::getProfileNames() const
{
    return cabinEQValueTreeManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEQValueTree>> StartupMVPAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
}

std::optional<juce::String> StartupMVPAudioProcessor::getLastSelectedProfileName()
{
    return cabinEQValueTreeManager.getLastSelectedProfileName();
}

void StartupMVPAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEQValueTreeManager.setLastSelectedProfileName (profileName);
}

void StartupMVPAudioProcessor::startSineSweep (float centerFreq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.setIsSweeping (true);
        playbackManager.startSineSweep (centerFreq,
                                        profile->get().getAmplCurve(),
                                        profile->get().getPanCurve());
    }
}

void StartupMVPAudioProcessor::updateSineSweep (float centerFreq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.setIsSweeping (true);
        playbackManager.updateSineSweep (centerFreq,
                                         profile->get().getAmplCurve(),
                                         profile->get().getPanCurve());
    }
}

void StartupMVPAudioProcessor::endSineSweep()
{
    playbackManager.setIsSweeping (false);
}

float StartupMVPAudioProcessor::getCurrSineSweepFreq()
{
    return playbackManager.getCurrSineSweepFreq();
}

void StartupMVPAudioProcessor::setReferenceVolume (float volume)
{
    playbackManager.setReferenceVolume (volume);
}

void StartupMVPAudioProcessor::setReferencePan (float pan)
{
    playbackManager.setReferencePan (pan);
}

void StartupMVPAudioProcessor::setReferenceVolume1 (float volume)
{
    playbackManager.setReferenceVolume1 (volume);
}

void StartupMVPAudioProcessor::setReferenceVolume2 (float volume)
{
    playbackManager.setReferenceVolume2 (volume);
}

void StartupMVPAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void StartupMVPAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

std::optional<std::reference_wrapper<CabinEQValueTree>> StartupMVPAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
}
