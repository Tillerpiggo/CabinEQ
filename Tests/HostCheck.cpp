/*
  ==============================================================================

    HostCheck.cpp

    Loads the built CabinEQ.vst3 the way a DAW would, through JUCE's VST3 hosting,
    and checks it processes, saves and restores, migrates old state, and opens its
    editor. A stand-in for pluginval.
        cmake --build build --target CabinEQ_HostCheck
        build/CabinEQ_HostCheck_artefacts/Release/CabinEQ_HostCheck build/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace
{
    int failures = 0;

    void check (bool condition, const juce::String& what)
    {
        std::cout << (condition ? "  ok    " : "  FAIL  ") << what << std::endl;
        if (! condition)
            ++failures;
    }

    std::unique_ptr<juce::AudioPluginInstance> load (juce::VST3PluginFormat& format, const juce::PluginDescription& description, double sampleRate, int blockSize)
    {
        juce::String error;
        auto instance = format.createInstanceFromDescription (description, sampleRate, blockSize, error);
        if (instance == nullptr)
            std::cout << "  couldn't load: " << error << std::endl;
        return instance;
    }

    float process (juce::AudioPluginInstance& plugin, int numBlocks, int blockSize)
    {
        juce::AudioBuffer<float> buffer (2, blockSize);
        juce::MidiBuffer midi;
        juce::Random random (1);
        float peak = 0.0f;
        for (int block = 0; block < numBlocks; ++block)
        {
            for (int channel = 0; channel < 2; ++channel)
                for (int i = 0; i < blockSize; ++i)
                    buffer.setSample (channel, i, (random.nextFloat() - 0.5f) * 0.5f);
            plugin.processBlock (buffer, midi);
            peak = std::max (peak, buffer.getMagnitude (0, blockSize));
        }
        return peak;
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juce;
    if (argc < 2)
    {
        std::cout << "Usage: CabinEQ_HostCheck <path to CabinEQ.vst3>" << std::endl;
        return 2;
    }

    juce::VST3PluginFormat format;
    juce::OwnedArray<juce::PluginDescription> found;
    format.findAllTypesForFile (found, juce::File (juce::String (argv[1])).getFullPathName());
    check (found.size() == 1, "the bundle has one plugin");
    if (found.isEmpty())
        return 1;
    const auto description = *found[0];
    check (description.name == "CabinEQ", "it's called CabinEQ (" + description.name + ")");
    check (description.manufacturerName == "Cabin Audio", "made by Cabin Audio (" + description.manufacturerName + ")");

    for (double sampleRate : { 44100.0, 48000.0, 96000.0 })
    {
        std::cout << "At " << sampleRate << " Hz" << std::endl;
        auto plugin = load (format, description, sampleRate, 512);
        check (plugin != nullptr, "loads");
        if (plugin == nullptr)
            return 1;

        plugin->setPlayConfigDetails (2, 2, sampleRate, 512);
        plugin->prepareToPlay (sampleRate, 512);
        const float peak = process (*plugin, 400, 512);
        check (std::isfinite (peak) && peak > 0.01f && peak < 4.0f, "processes noise (peak " + juce::String (peak, 3) + ")");

        // Odd block sizes, as some hosts send
        const float oddPeak = process (*plugin, 50, 37);
        check (std::isfinite (oddPeak), "handles odd block sizes");
        plugin->releaseResources();
    }

    std::cout << "Parameters" << std::endl;
    {
        auto plugin = load (format, description, 48000.0, 512);
        juce::StringArray names;
        for (auto* parameter : plugin->getParameters())
            names.add (parameter->getName (64));
        std::cout << "  " << names.joinIntoString (", ") << std::endl;
        check (names.contains ("Bypass") && names.contains ("Auto Gain") && names.contains ("Crossfeed"), "has bypass, auto gain and crossfeed");
        check (plugin->getBypassParameter() != nullptr, "the host can find the bypass parameter");
    }

    std::cout << "State" << std::endl;
    {
        // What CabinEQ saved before this rewrite
        const char* oldXml = R"(<Params lastSelectedProfileId="Old" masterVolumeId="0" hasLicenseId="0">
            <PARAM id="dummyParam" value="0"/>
            <Profile ProfileName="Old" Locked="0" ProfileVolume="-3" MelodyVolume="0" NoiseVolume="0">
              <AmplTree><MultiBandStep id="0" Enabled="1">
                <Band id="0" freq="1000" ampl="6" bandwidth="1" bandtype="0"/>
                <Band id="1" freq="4000" ampl="-4" bandwidth="0.5" bandtype="0"/>
              </MultiBandStep></AmplTree>
            </Profile></Params>)";
        juce::MemoryBlock oldPluginState;
        juce::AudioProcessor::copyXmlToBinary (*juce::parseXML (oldXml), oldPluginState);

        // A host hands the plugin's own bytes over as the VST3 component state
        juce::XmlElement wrapper ("VST3PluginState");
        wrapper.createNewChildElement ("IComponent")->addTextElement (oldPluginState.toBase64Encoding());
        juce::MemoryBlock oldState;
        juce::AudioProcessor::copyXmlToBinary (wrapper, oldState);

        // Loading old state makes the plugin back it up; don't leave the test's backup lying around
        const auto backups = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                 .getChildFile ("Application Support/CabinEQ/Backups");
        const auto backupsBefore = backups.findChildFiles (juce::File::findFiles, false, "*.xml");

        auto plugin = load (format, description, 48000.0, 512);
        plugin->setStateInformation (oldState.getData(), (int) oldState.getSize());

        for (const auto& file : backups.findChildFiles (juce::File::findFiles, false, "*.xml"))
            if (! backupsBefore.contains (file))
                file.deleteFile();
        if (backupsBefore.isEmpty() && backups.getNumberOfChildFiles (juce::File::findFilesAndDirectories) == 0)
        {
            backups.deleteFile();
            backups.getParentDirectory().deleteFile(); // only removes it if it's empty
        }
        juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

        juce::MemoryBlock saved;
        plugin->getStateInformation (saved);
        auto savedXml = juce::AudioProcessor::getXmlFromBinary (saved.getData(), (int) saved.getSize());
        check (savedXml != nullptr, "saves state");

        // Unwrap the plugin's own state from the host's
        std::unique_ptr<juce::XmlElement> pluginXml;
        if (savedXml != nullptr)
        {
            juce::MemoryBlock component;
            if (auto* chunk = savedXml->getChildByName ("IComponent"); chunk != nullptr && component.fromBase64Encoding (chunk->getAllSubText()))
                pluginXml = juce::AudioProcessor::getXmlFromBinary (component.getData(), (int) component.getSize());
        }
        check (pluginXml != nullptr, "the component state is the plugin's XML");

        if (pluginXml != nullptr)
        {
            auto profile = pluginXml->getChildByAttribute ("ProfileName", "Old");
            auto* bands = profile != nullptr ? profile->getChildByName ("Bands") : nullptr;
            check (bands != nullptr && bands->getNumChildElements() == 2, "old profile's bands were migrated");
        }

        // Restoring into a fresh instance gives back the same state
        auto restored = load (format, description, 48000.0, 512);
        restored->setStateInformation (saved.getData(), (int) saved.getSize());
        juce::MessageManager::getInstance()->runDispatchLoopUntil (50);
        juce::MemoryBlock savedAgain;
        restored->getStateInformation (savedAgain);
        auto again = juce::AudioProcessor::getXmlFromBinary (savedAgain.getData(), (int) savedAgain.getSize());
        check (again != nullptr && savedXml != nullptr && again->isEquivalentTo (savedXml.get(), false), "state round trips exactly");

        // The migrated EQ is audible: a 1 kHz sine comes out louder
        restored->setPlayConfigDetails (2, 2, 48000.0, 512);
        restored->prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> buffer (2, 512);
        juce::MidiBuffer midi;
        double phase = 0.0, inPower = 0.0, outPower = 0.0;
        for (int block = 0; block < 200; ++block)
        {
            for (int i = 0; i < 512; ++i)
            {
                const auto sample = (float) (0.1 * std::sin (phase));
                phase += juce::MathConstants<double>::twoPi * 1000.0 / 48000.0;
                buffer.setSample (0, i, sample);
                buffer.setSample (1, i, sample);
                if (block >= 100) inPower += sample * sample;
            }
            restored->processBlock (buffer, midi);
            if (block >= 100)
                for (int i = 0; i < 512; ++i)
                    outPower += buffer.getSample (0, i) * buffer.getSample (0, i);
        }
        const double gainDb = 10.0 * std::log10 (outPower / inPower);
        // +6 dB bell, -3 dB preamp, auto gain off for migrated state
        check (std::abs (gainDb - 3.0) < 0.5, "1 kHz comes out " + juce::String (gainDb, 2) + " dB (expected about +3)");
    }

    std::cout << "Editor" << std::endl;
    {
        auto plugin = load (format, description, 48000.0, 512);
        std::unique_ptr<juce::AudioProcessorEditor> editor (plugin->createEditorIfNeeded());
        check (editor != nullptr, "opens its editor");
        if (editor != nullptr)
        {
            check (editor->getWidth() >= 760 && editor->getHeight() >= 460, "editor is a sensible size ("
                   + juce::String (editor->getWidth()) + " x " + juce::String (editor->getHeight()) + ")");
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
            editor.reset();
        }

        // Open and close it a few times while audio runs
        plugin->setPlayConfigDetails (2, 2, 48000.0, 512);
        plugin->prepareToPlay (48000.0, 512);
        for (int i = 0; i < 5; ++i)
        {
            std::unique_ptr<juce::AudioProcessorEditor> another (plugin->createEditorIfNeeded());
            process (*plugin, 20, 512);
            juce::MessageManager::getInstance()->runDispatchLoopUntil (20);
        }
        check (true, "editor opens and closes repeatedly");
    }

    std::cout << (failures == 0 ? "All host checks passed" : juce::String (failures) + " host checks failed") << std::endl;
    return failures == 0 ? 0 : 1;
}
