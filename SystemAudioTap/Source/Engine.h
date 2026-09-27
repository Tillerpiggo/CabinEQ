#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_events/juce_events.h>

#include "AudioDevices.h"
#include "SystemAudioTap.h"

// Runs system audio through the plugin, and keeps checking that it's actually working:
// that the plugin loaded, macOS lets us hear the audio, we're playing to something you can
// hear, and audio is really going in and coming out. When devices change it follows them.
// Everything here is for the message thread, apart from the audio callback.
class Engine : private juce::Timer
{
public:
    // With startAudio false it only looks (for --snapshot): no permission prompt, no tap
    explicit Engine (bool startAudio = true);
    ~Engine() override;

    static juce::File getLogFile(); // ~/Library/Logs/CabinEQ System.log, a line per status change

    //==============================================================================
    // The plugin
    juce::AudioPluginInstance* getPlugin() const { return plugin.get(); }
    juce::File getPluginFile() const { return pluginFile; }
    juce::String getPluginError() const { return pluginError; }
    void setPluginFile (const juce::File& file);   // loads it and reconnects
    static juce::File getDefaultPluginFile();

    // Permission to hear other apps' audio
    SystemAudioTap::Permission getPermission() const { return permission; }
    bool isAskingForPermission() const { return askingForPermission; }
    bool hasAskedForPermission() const { return permissionAsked; }
    void askForPermission();   // shows the macOS prompt if it hasn't been answered, then connects

    // Where the processed audio goes. An empty choice follows the Mac's output, but never to a
    // virtual device, since nobody would hear it.
    juce::String getOutputChoice() const { return outputChoice; }
    void setOutputChoice (const juce::String& uid);
    const std::vector<AudioDevices::OutputDevice>& getDevices() const { return devices; }
    juce::String getPlayingToName() const;
    juce::String getPlayingToUID() const;
    std::optional<AudioDevices::OutputDevice> getMacOutput() const;
    void makeMacOutput (const juce::String& uid);

    // Other apps that also process system audio
    const std::vector<AudioDevices::RunningApp>& getConflictingApps() const { return conflictingApps; }
    void quitConflictingApp (int pid);

    //==============================================================================
    struct Status
    {
        enum class Level { working, idle, busy, warning, problem };
        Level level = Level::busy;
        juce::String headline, detail;
        bool operator!= (const Status& other) const { return level != other.level || headline != other.headline || detail != other.detail; }
    };
    Status getStatus() const { return status; }
    bool isConnected() const { return tap.isRunning(); }
    juce::String getConnectError() const { return connectError; }

    // Meters, in dBFS, smoothed for display
    float getInputLevel() const { return inputLevel; }
    float getOutputLevel() const { return outputLevel; }

    // Plays a short sound from another process and checks it came through
    enum class TestState { notRun, playing, passed, failed };
    void playTestSound();
    TestState getTestState() const { return testState; }
    juce::String getTestResult() const { return testResult; }

    // Whether the setup checklist has been gone through once
    bool hasSeenSetup() const;
    void setHasSeenSetup();

    std::function<void()> onChange;   // status, meters or devices changed

private:
    void loadPlugin();
    bool canConnect() const;
    void connect();
    void disconnect();
    juce::String chooseOutput() const;
    void process (const float* const* in, float* const* out, int numFrames);
    void timerCallback() override;
    void devicesChanged();
    void updateStatus();
    void finishTest();

    mutable juce::ApplicationProperties settings;
    juce::AudioPluginFormatManager formats;   // declared before `plugin` so it outlives it
    std::unique_ptr<juce::AudioPluginInstance> plugin;
    juce::File pluginFile;
    juce::String pluginError;

    SystemAudioTap tap;
    bool prepared = false;
    double preparedSampleRate = 0;
    int blockSize = 512;
    juce::AudioBuffer<float> work;
    juce::MidiBuffer midi;
    std::atomic<int> slowCallbacks { 0 };
    double callbackBudgetSeconds = 0.01;
    juce::String connectError;

    SystemAudioTap::Permission permission = SystemAudioTap::Permission::unknown;
    bool askingForPermission = false;
    bool permissionAsked = false;

    juce::String outputChoice;
    std::vector<AudioDevices::OutputDevice> devices;
    std::vector<AudioDevices::RunningApp> conflictingApps;
    std::unique_ptr<AudioDevices::ChangeListener> deviceListener;

    // What the timer has seen
    double lastCallbackTime = 0, lastInputTime = 0, lastOutputTime = 0, lastReconnectTime = 0;
    double otherAppPlayingSince = 0, lastDropoutTime = 0, lastSlowCheck = 0, lastPoll = 0;
    bool otherAppPlaying = false;
    float inputLevel = -100, outputLevel = -100;
    Status status;

    TestState testState = TestState::notRun;
    juce::String testResult;
    std::unique_ptr<juce::ChildProcess> testPlayer;
    double testStartedAt = 0;
    float testInputPeak = 0, testOutputPeak = 0;

    bool audioAllowed = true;
    std::unique_ptr<juce::FileLogger> log;

    std::shared_ptr<bool> alive = std::make_shared<bool> (true);
};
