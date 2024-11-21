/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "CabinEqAudioProcessor.h"
#include "CabinEqProcessorEditor.h"
#include <chrono>

//==============================================================================
CabinEqAudioProcessor::CabinEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters (*this, nullptr, "Params", createParameterLayout()),
                          cabinEqProfileManager (parameters)

#endif
{
}

CabinEqAudioProcessor::~CabinEqAudioProcessor()
{
}

//==============================================================================
const juce::String CabinEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CabinEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CabinEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CabinEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CabinEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CabinEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CabinEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CabinEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void CabinEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CabinEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    playbackManager.prepare (spec);
    
    // Define glyphs
    Glyph circleGlyph ({
        Stroke ({
            { 0.5, 0 }, { 0.35, 0.35 }, { 0, 0.5 }, { -0.35, 0.35 },
            { -0.5, 0 }, { -0.35, -0.35 }, { 0, -0.5 }, { 0.35, -0.35 },
            { 0.5, 0 } // Closing the circle
        }),
        Stroke ({
            { 0.05, 0 }, { 0, 0.05 }, { -0.05, 0 }, { 0, -0.05 },
            { 0.05, 0 } // Closing the dot
        })
    });
    Glyph mGlyph ({ Stroke ({{ -0.15, -0.5 }, { -0.15, 0 }, { -0.15, 0.5 }, { 0, -0.4 }, { 0.15, 0.5 }, { 0.15, 0 }, { 0.15, -0.5 }}) });
    Glyph diagonalGlyph ({ Stroke ({{ -1, 0.5 }, { -0.5, 1 }}), Stroke ({{ -1, 0 }, { 0, 1 }}), Stroke ({{ -1, -0.5 }, { 0.5, 1 }})});
    Glyph wiggleGlyph ({ Stroke ({{ -1, -0.3 }, { -1, 0.3 }, { -1, -0.3 }}), Stroke ({{ 1, -0.3 }, { 1, 0.3 }, { 1, -0.3 }})});
    Glyph dotsGridGlyph ({
        // First row
        Stroke ({{ -0.5, 0.5 }, { -0.5, 0.5 }}),
        Stroke ({{ 0, 0.5 }, { 0, 0.5 }}),
        Stroke ({{ 0.5, 0.5 }, { 0.5, 0.5 }}),

        // Second row
        Stroke ({{ -0.5, 0 }, { -0.5, 0 }}),
        Stroke ({{ 0, 0 }, { 0, 0 }}),
        Stroke ({{ 0.5, 0 }, { 0.5, 0 }}),

        // Third row
        Stroke ({{ -0.5, -0.5 }, { -0.5, -0.5 }}),
        Stroke ({{ 0, -0.5 }, { 0, -0.5 }}),
        Stroke ({{ 0.5, -0.5 }, { 0.5, -0.5 }})
    });
    Glyph dotsGlyph ({ Stroke ({{ -1, 0 }, { -1, 0 }}), Stroke ({{ -0.5, 0 }, { -0.5, 0 }}), Stroke ({{ 0, 0 }, { 0, 0 }}), Stroke ({{ 0.5, 0 }, { 0.5, 0 }}), Stroke ({{ 1, 0 }, { 1, 0 }})});
    Glyph dotsGlyph2 ({ Stroke ({{ -1, -0.8 }, { -1, -0.8 }}), Stroke ({{ 0, -0.8 }, { 0, -0.8 }}), Stroke ({{ 1, -0.8 }, { 1, -0.8 }})});
    Glyph dotsGlyph3 ({ Stroke ({{ -1, 0.8 }, { -1, 0.8 }}), Stroke ({{ 0, 0.8 }, { 0, 0.8 }}), Stroke ({{ 1, 0.8 }, { 1, 0.8 }})});
    Glyph linesGlyph ({ Stroke ({{ -1, -1 }, { 1, -1 }}), Stroke ({{ -1, -0.5 }, { 1, -0.5 }}), Stroke ({{ -1, 0 }, { 1, 0 }}), Stroke ({{ -1, 0.5 }, { 1, 0.5 }}), Stroke ({{ -1, 1 }, { 1, 1 }})});
    Glyph squareGlyph ({ Stroke ({{ -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }})});
    Glyph rectGlyph ({ Stroke ({{ -1, -0.3 }, { 1, -0.3 }, { 1, 0.3 }, { -1, 0.3 }, { -1, -0.3 }})});
    Glyph triangleGlyph ({ Stroke ({{ -1, -1 }, { 1, -1 }, { 0, 1 }, { -1, -1 }})});
    Glyph xGlyph ({ Stroke ({{ -1, -1 }, { 1, 1 }}), Stroke ({{ 1, -1 }, { -1, 1 }})});
    Glyph diamondPlusGlyph ({ Stroke ({{ 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }}), Stroke ({{ 0, -1 }, { 0, 1 }}), Stroke ({{ -1, 0 }, { 1, 0 }})});
    Glyph fourXGlyph ({ Stroke ({{ -1, -1 }, { 0, 0 }}), Stroke ({{ 0, -1 }, { -1, 0 }}),
        Stroke ({{ 0, -1 }, { 1, 0 }}), Stroke ({{ 1, -1 }, { 0, 0 }}),
        Stroke ({{ -1, 0 }, { 0, 1 }}), Stroke ({{ 0, 0 }, { -1, 1 }}),
        Stroke ({{ 0, 0 }, { 1, 1 }}), Stroke ({{ 1, 0 }, { 0, 1 }})});
    Glyph triangleStrokes ({ Stroke ({{ -1, -1 }, { -0.66, 1 }, { -0.33, -1 }, { 0, 1 }, { 0.33, -1 }, { 0.66, 1 }, { 1, -1 }})});
    Glyph edgeStrokes ({ Stroke ({{ -1, 1 }, { -0.5, 0.5 }, { -1, 0 }, { -0.5, -0.5 }, { -1, -1 }}),
        Stroke ({{ 1, 1 }, { 0.5, 0.5 }, { 1, 0 }, { 0.5, -0.5 }, { 1, -1 }})});
    Glyph spiralGlyph ({
        Stroke ({{ -1, -1 }, { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 }}),
        Stroke ({{ 1, 1 }, { 1, 0 }, { 0, 0 }, { 0, -1 }, { -1, -1 }})
    });
    Glyph complexFractalGlyph ({
        Stroke ({{ -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 }}),  // Outer square boundary
        Stroke ({{ -0.5, -0.5 }, { -0.5, 0.5 }, { 0.5, 0.5 }, { 0.5, -0.5 }, { -0.5, -0.5 }}),  // Inner square
        Stroke ({{ -1, 0 }, { 1, 0 }}),  // Horizontal line through center
        Stroke ({{ 0, -1 }, { 0, 1 }}),  // Vertical line through center
        Stroke ({{ -1, -0.5 }, { 1, -0.5 }}),  // Additional horizontal lines
        Stroke ({{ -1, 0.5 }, { 1, 0.5 }}),
        Stroke ({{ -0.5, -1 }, { -0.5, 1 }}),  // Additional vertical lines
        Stroke ({{ 0.5, -1 }, { 0.5, 1 }})
    });
    
    Glyph floatingSquaresGlyph ({
        Stroke ({{ -0.8, -0.8 }, { -0.6, -0.8 }, { -0.6, -0.6 }, { -0.8, -0.6 }, { -0.8, -0.8 }}),  // Bottom-left square
        Stroke ({{ 0.6, -0.8 }, { 0.8, -0.8 }, { 0.8, -0.6 }, { 0.6, -0.6 }, { 0.6, -0.8 }}),    // Bottom-right square
        Stroke ({{ -0.8, 0.6 }, { -0.6, 0.6 }, { -0.6, 0.8 }, { -0.8, 0.8 }, { -0.8, 0.6 }}),    // Top-left square
        Stroke ({{ 0.6, 0.6 }, { 0.8, 0.6 }, { 0.8, 0.8 }, { 0.6, 0.8 }, { 0.6, 0.6 }}),        // Top-right square
        Stroke ({{ -0.1, -0.1 }, { 0.1, -0.1 }, { 0.1, 0.1 }, { -0.1, 0.1 }, { -0.1, -0.1 }})   // Center square
    });
    
    Glyph distributedSquaresGlyph ({
        Stroke ({{ -0.9, -0.9 }, { -0.7, -0.9 }, { -0.7, -0.7 }, { -0.9, -0.7 }, { -0.9, -0.9 }}),  // Bottom-left
        Stroke ({{ 0.7, -0.9 }, { 0.9, -0.9 }, { 0.9, -0.7 }, { 0.7, -0.7 }, { 0.7, -0.9 }}),    // Bottom-right
        Stroke ({{ -0.9, 0.7 }, { -0.7, 0.7 }, { -0.7, 0.9 }, { -0.9, 0.9 }, { -0.9, 0.7 }}),    // Top-left
        Stroke ({{ 0.7, 0.7 }, { 0.9, 0.7 }, { 0.9, 0.9 }, { 0.7, 0.9 }, { 0.7, 0.7 }}),        // Top-right
        Stroke ({{ -0.2, -0.3 }, { 0.0, -0.3 }, { 0.0, -0.1 }, { -0.2, -0.1 }, { -0.2, -0.3 }}), // Center-left
        Stroke ({{ 0.2, 0.2 }, { 0.4, 0.2 }, { 0.4, 0.4 }, { 0.2, 0.4 }, { 0.2, 0.2 }}),        // Center-right
        Stroke ({{ -0.5, 0.0 }, { -0.3, 0.0 }, { -0.3, 0.2 }, { -0.5, 0.2 }, { -0.5, 0.0 }}),   // Middle-left
        Stroke ({{ 0.3, -0.5 }, { 0.5, -0.5 }, { 0.5, -0.3 }, { 0.3, -0.3 }, { 0.3, -0.5 }})    // Middle-right
    });
    Glyph graphPaperSquaresGlyph ({
        Stroke ({{ -0.8, -0.8 }, { -0.8, 0.0 }, { 0.0, 0.0 }, { 0.0, -0.8 }, { -0.8, -0.8 }}),  // Bottom-left square
        Stroke ({{ -0.4, -0.4 }, { -0.4, 0.4 }, { 0.4, 0.4 }, { 0.4, -0.4 }, { -0.4, -0.4 }}),  // Center square
        Stroke ({{ 0.0, 0.0 }, { 0.0, 0.8 }, { 0.8, 0.8 }, { 0.8, 0.0 }, { 0.0, 0.0 }}),       // Top-right square
        Stroke ({{ -0.8, 0.2 }, { -0.8, 1.0 }, { 0.0, 1.0 }, { 0.0, 0.2 }, { -0.8, 0.2 }}),    // Top-left square
        Stroke ({{ 0.2, -1.0 }, { 0.2, -0.2 }, { 1.0, -0.2 }, { 1.0, -1.0 }, { 0.2, -1.0 }})   // Bottom-right square
    });
    
    Glyph triforceGlyph({
        Stroke({{-0.5, 0}, {-1, -0.866}, {0, -0.866}, {-0.5, 0}}), // Bottom-left triangle
        Stroke({{0.5, 0}, {0, -0.866}, {1, -0.866}, {0.5, 0}}),    // Bottom-right triangle
        Stroke({{0, 0.866}, {-0.5, 0}, {0.5, 0}, {0, 0.866}})      // Top triangle
    });
    
    Glyph gridGlyph({
        // Row 1
        Stroke({{-0.9, 0.9}, {-0.7, 0.9}, {-0.7, 0.7}, {-0.9, 0.7}, {-0.9, 0.9}}), // Top-left square
        Stroke({{-0.4, 0.9}, {-0.2, 0.9}, {-0.2, 0.7}, {-0.4, 0.7}, {-0.4, 0.9}}), // Top-middle square
        Stroke({{0.1, 0.9}, {0.3, 0.9}, {0.3, 0.7}, {0.1, 0.7}, {0.1, 0.9}}),     // Top-right square

        // Row 2
        Stroke({{-0.9, 0.4}, {-0.7, 0.4}, {-0.7, 0.2}, {-0.9, 0.2}, {-0.9, 0.4}}), // Middle-left square
        Stroke({{-0.4, 0.4}, {-0.2, 0.4}, {-0.2, 0.2}, {-0.4, 0.2}, {-0.4, 0.4}}), // Center square
        Stroke({{0.1, 0.4}, {0.3, 0.4}, {0.3, 0.2}, {0.1, 0.2}, {0.1, 0.4}}),     // Middle-right square

        // Row 3
        Stroke({{-0.9, -0.1}, {-0.7, -0.1}, {-0.7, -0.3}, {-0.9, -0.3}, {-0.9, -0.1}}), // Bottom-left square
        Stroke({{-0.4, -0.1}, {-0.2, -0.1}, {-0.2, -0.3}, {-0.4, -0.3}, {-0.4, -0.1}}), // Bottom-middle square
        Stroke({{0.1, -0.1}, {0.3, -0.1}, {0.3, -0.3}, {0.1, -0.3}, {0.1, -0.1}})      // Bottom-right square
    });
    
    glyphManager.addGlyphs ({ dotsGridGlyph, circleGlyph, mGlyph, diagonalGlyph, wiggleGlyph, dotsGlyph, dotsGlyph2, dotsGlyph3, linesGlyph, squareGlyph, rectGlyph, triangleGlyph, xGlyph, diamondPlusGlyph, fourXGlyph, triangleStrokes, edgeStrokes, spiralGlyph, complexFractalGlyph, floatingSquaresGlyph, distributedSquaresGlyph, graphPaperSquaresGlyph, triforceGlyph, gridGlyph });
    playbackManager.setGlyph (getCurrGlyph());
}

void CabinEqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CabinEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CabinEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
bool CabinEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CabinEqAudioProcessor::createEditor()
{
    return new CabinEqProcessorEditor (*this);
}

//==============================================================================
void CabinEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = parameters.copyState();
    std::unique_ptr <juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CabinEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr <juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get())
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            cabinEqProfileManager.initProfiles();
            
            if (! hasLoadedData)
            {
                for (auto listener : listeners)
                    if (listener != nullptr)
                        listener->didLoadData();
                profileId = getLastSelectedProfileName().value_or ("NO_PROFILE");
                hasLoadedData = true;
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CabinEqAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout CabinEqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::NormalisableRange<float> range (-1.0f, 1.0f, 0.01f);
    
    juce::String paramID = "dummyParam";
    return { std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (paramID, 1), paramID, range, 0.0f) };
}

//==============================================================================
void CabinEqAudioProcessor::setVolume (float volume)
{
    playbackManager.setVolume (volume);
    updateFilter();
}

void CabinEqAudioProcessor::setIsFilterOn (bool isFilterOn)
{
    playbackManager.setIsFilterOn (isFilterOn);
}

void CabinEqAudioProcessor::setIsPlaying (bool isPlaying)
{
    playbackManager.setIsPlayingNoise (isPlaying);
}

void CabinEqAudioProcessor::setSpeedFactor (float speedFactor)
{
    playbackManager.setSpeedFactor (speedFactor);
}

void CabinEqAudioProcessor::setBandwidth (float bandwidth)
{
    playbackManager.setBandwidth (bandwidth);
}

void CabinEqAudioProcessor::addProfile (juce::String profileName)
{
    cabinEqProfileManager.addProfile (profileName);
}

void CabinEqAudioProcessor::addDuplicateProfile (juce::String profileName, juce::String oldProfileName)
{
    cabinEqProfileManager.addDuplicateProfile (profileName, oldProfileName);
}

void CabinEqAudioProcessor::removeProfile (juce::String profileName)
{
    cabinEqProfileManager.removeProfile (profileName);
}

void CabinEqAudioProcessor::renameProfile (juce::String profileName, juce::String newProfileName)
{
    cabinEqProfileManager.renameProfile (profileName, newProfileName);
}

void CabinEqAudioProcessor::setProfileVolume (juce::String profileName, float masterVolume)
{
    cabinEqProfileManager.setProfileVolume (profileName, masterVolume);
}

const std::vector<juce::String> CabinEqAudioProcessor::getProfileNames() const
{
    return cabinEqProfileManager.getProfileNames();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::getProfileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}

BandProfile CabinEqAudioProcessor::getBandProfile()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        return profile->get().getBandProfile();
    return BandProfile ({}, 0.0f, 0.0f, 0.0f);
}

std::optional<juce::String> CabinEqAudioProcessor::getLastSelectedProfileName()
{
    return cabinEqProfileManager.getLastSelectedProfileName();
}

void CabinEqAudioProcessor::setLastSelectedProfileName (juce::String profileName)
{
    cabinEqProfileManager.setLastSelectedProfileName (profileName);
    profileId = profileName;
}

void CabinEqAudioProcessor::updateFilter()
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
        playbackManager.updateFilterWithBandProfile (profile->get().getBandProfile());
}

int CabinEqAudioProcessor::addBand (const float freq, const float ampl, const float bandwidth, const Band::Type type)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        int bandId = profile->get().addBand (freq, ampl, bandwidth, type);
        updateFilter();
        return bandId;
    }
        
    return -1;
}

void CabinEqAudioProcessor::updateBand (const int id, const float freq, const float ampl, const float bandwidth, const Band::Type type)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        profile->get().updateBand (id, freq, ampl, bandwidth, type);
        updateFilter();
    }
}

void CabinEqAudioProcessor::removeBand (const int id)
{
    auto profile = profileNamed (profileId);
    if (profile.has_value())
    {
        profile->get().removeBand (id);
        updateFilter();
    }
}

void CabinEqAudioProcessor::addListener (Listener* listener)
{
    this->listeners.push_back (listener);
}

void CabinEqAudioProcessor::removeListener()
{
    // VERY BAD FIX THIS: eh whatever
}

bool CabinEqAudioProcessor::hasNextGlyph()
{
    return glyphManager.hasNext();
}

bool CabinEqAudioProcessor::hasPrevGlyph()
{
    return glyphManager.hasPrev();
}

void CabinEqAudioProcessor::goToNextGlyph()
{
    glyphManager.goToNext();
    playbackManager.setGlyph (getCurrGlyph());
}

void CabinEqAudioProcessor::goToPrevGlyph()
{
    glyphManager.goToPrev();
    playbackManager.setGlyph (getCurrGlyph());
}

Glyph CabinEqAudioProcessor::getCurrGlyph()
{
    std::cout << "Getting curr glyph" << std::endl;
    return glyphManager.getCurrGlyph();
}

void CabinEqAudioProcessor::setSizeFactor (float sizeFactor)
{
    glyphManager.setSizeFactor (sizeFactor);
    playbackManager.setSizeFactor (sizeFactor);
}

void CabinEqAudioProcessor::setCenterPos (juce::Point<float> centerPos)
{
    glyphManager.setCenterPos (centerPos);
    playbackManager.setCenterPos (centerPos);
}

float CabinEqAudioProcessor::getSizeFactor() const
{
    return glyphManager.getSizeFactor();
}

juce::Point<float> CabinEqAudioProcessor::getCenterPos() const
{
    return glyphManager.getCenterPos();
}

float CabinEqAudioProcessor::getCurrPlayingTime()
{
    return playbackManager.getCurrPlayingTime();
}

std::optional<std::reference_wrapper<CabinEqProfile>> CabinEqAudioProcessor::profileNamed (juce::String profileName) const
{
    return cabinEqProfileManager.getProfileNamed (profileName);
}
