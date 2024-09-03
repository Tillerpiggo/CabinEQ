/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CabinEQAudioProcessor.h"
#include "CabinEQProcessorEditor.h"
#include <chrono>

//==============================================================================
CabinEQAudioProcessor::CabinEQAudioProcessor()
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

CabinEQAudioProcessor::~CabinEQAudioProcessor()
{
}

//==============================================================================
const juce::String CabinEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CabinEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CabinEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CabinEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CabinEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CabinEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CabinEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CabinEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CabinEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void CabinEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CabinEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
}

void CabinEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CabinEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CabinEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
bool CabinEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CabinEQAudioProcessor::createEditor()
{
    return new CabinEQProcessorEditor (*this);
}

//==============================================================================
void CabinEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CabinEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new CabinEQAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CabinEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void CabinEQAudioProcessor::applyCurve (int fftSize, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updateFilterWithCurves (profile->get().getAmplCurve(), profile->get().getPanCurve(), fftSize);
    }
        
}

void CabinEQAudioProcessor::setIsProcessing (bool isProcessing)
{
    playbackManager.setIsProcessing (isProcessing);
}

void CabinEQAudioProcessor::setBypassBalance (float balance)
{
    playbackManager.setDryWetVolumeBalance (balance);
}

void CabinEQAudioProcessor::setWetVolume (float wetVolume)
{
    playbackManager.setWetVolume (wetVolume);
}

void CabinEQAudioProcessor::setDryVolume (float dryVolume)
{
    playbackManager.setDryVolume (dryVolume);
}

std::optional<std::reference_wrapper<Curve>> CabinEQAudioProcessor::getAmplCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplCurve();
    std::cout << "unable to get ampl curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

std::optional<std::reference_wrapper<Curve>> CabinEQAudioProcessor::getPanCurve (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanCurve();
    std::cout << "unable to get pan curve in pluginProcessor for profile named " << profileName << std::endl;
    return std::nullopt;
}

const std::vector<CurvePt> CabinEQAudioProcessor::getAmplPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplPts();
    return {};
}

const std::vector<CurvePt> CabinEQAudioProcessor::getPanPts (juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanPts();
    return {};
}

const std::optional<CurvePt> CabinEQAudioProcessor::getAmplPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getAmplPtWithId (id);
    return std::nullopt;
}

const std::optional<CurvePt> CabinEQAudioProcessor::getPanPtWithId (int id, juce::String profileName) const
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().getPanPtWithId (id);
    return std::nullopt;
}

int CabinEQAudioProcessor::addAmplPt (const float freq, const float ampl, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addAmplPt (freq, ampl);
    return -1;
}

int CabinEQAudioProcessor::addPanPt (const float freq, const float pan, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        return profile->get().addPanPt (freq, pan);
    return -1;
}

void CabinEQAudioProcessor::removeAmplPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removeAmplPt (id);
}

void CabinEQAudioProcessor::removePanPt (const int id, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().removePanPt (id);
}

void CabinEQAudioProcessor::updateAmplPt (const int id, const float freq, const float ampl, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updateAmplPt (id, freq, ampl);
}

void CabinEQAudioProcessor::updatePanPt (const int id, const float freq, const float pan, juce::String profileName)
{
    currProfileName = profileName; // super hacky
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().updatePanPt (id, freq, pan);
}

void CabinEQAudioProcessor::clearEQNodes (juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
        profile->get().resetNodes();
}

void CabinEQAudioProcessor::startPlayingFreq (float freq, juce::String profileName)
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
void CabinEQAudioProcessor::startPlayingReferenceFreqs()
{
    playbackManager.setIsCalibrating (true);
    playbackManager.startPlayingReferenceFreqs();
}

void CabinEQAudioProcessor::updatePlayingReferenceFreqs()
{
    playbackManager.updatePlayingReferenceFreqs();
}

void CabinEQAudioProcessor::updatePlayingFreq (float freq, juce::String profileName)
{
    auto profile = profileNamed (profileName);
    if (profile.has_value())
    {
        playbackManager.updatePlayingFreq (freq,
                                          profile->get().getAmplCurve(),
                                          profile->get().getPanCurve());
    }
}

void CabinEQAudioProcessor::endCalibratingEQNode()
{
    playbackManager.setIsCalibrating (false);
}

float CabinEQAudioProcessor::getCurrPlayingFreq()
{
    return playbackManager.getCurrPlayingFreq();
}

void CabinEQAudioProcessor::startTestingAt (float freq, juce::String profileName)
{
    auto curve = getAmplCurve (profileName); // hacky for now
    if (curve.has_value())
        playbackManager.startTestingFreq (freq, curve->get());
}

void CabinEQAudioProcessor::updateTestingAt (float freq, juce::String profileName)
{
    auto curve = getAmplCurve (profileName); // hacky for now
    if (curve.has_value())
        playbackManager.updateTestingFreq (freq, curve->get());
}

void CabinEQAudioProcessor::endTesting()
{
    playbackManager.stopTestingFreq();
}

float CabinEQAudioProcessor::getCurrTestingFreq()
{
    return playbackManager.getCurrTestingFreq();
}

void CabinEQAudioProcessor::addProfile (juce::String profileName)
{
    cabinEQValueTreeManager.addProfile (profileName);
}

void CabinEQAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEQValueTreeManager.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEQAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEQValueTreeManager.removeProfile (profileName);
}

const std::vector<juce::String> CabinEQAudioProcessor::getProfileNames() const
{
    return cabinEQValueTreeManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEQValueTree>> CabinEQAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
}

std::optional<juce::String> CabinEQAudioProcessor::getLastSelectedProfileName()
{
    return cabinEQValueTreeManager.getLastSelectedProfileName();
}

void CabinEQAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEQValueTreeManager.setLastSelectedProfileName (profileName);
}

void CabinEQAudioProcessor::startSineSweep (float centerFreq, juce::String profileName)
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

void CabinEQAudioProcessor::updateSineSweep (float centerFreq, juce::String profileName)
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

void CabinEQAudioProcessor::endSineSweep()
{
    playbackManager.setIsSweeping (false);
}

float CabinEQAudioProcessor::getCurrSineSweepFreq()
{
    return playbackManager.getCurrSineSweepFreq();
}

void CabinEQAudioProcessor::setReferenceVolume (float volume)
{
    playbackManager.setReferenceVolume (volume);
}

void CabinEQAudioProcessor::setReferencePan (float pan)
{
    playbackManager.setReferencePan (pan);
}

void CabinEQAudioProcessor::setReferenceVolume1 (float volume)
{
    playbackManager.setReferenceVolume1 (volume);
}

void CabinEQAudioProcessor::setReferenceVolume2 (float volume)
{
    playbackManager.setReferenceVolume2 (volume);
}

void CabinEQAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEQAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

std::optional<std::reference_wrapper<CabinEQValueTree>> CabinEQAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEQValueTreeManager.getProfileNamed (profileName);
}
