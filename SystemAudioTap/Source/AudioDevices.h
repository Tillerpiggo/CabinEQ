#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

// What CabinEQ System needs to know about the Mac's audio setup, from Core Audio and the
// list of running apps. Everything here is for the message (main) thread.
namespace AudioDevices
{
    struct OutputDevice
    {
        std::string uid, name;
        bool isVirtual = false;   // BlackHole, eqMac, Loopback and the like: nobody hears these directly
        bool isBuiltIn = false;
        bool isBluetooth = false;
    };

    // Real and virtual output devices, in Core Audio's order. Leaves out aggregate devices.
    std::vector<OutputDevice> getOutputDevices();

    std::string getDefaultOutputUID();
    bool setDefaultOutput (const std::string& uid);   // also sets the "system" output, for alerts

    // The real device CabinEQ System should play to when the Mac's output is a virtual one:
    // Bluetooth or USB headphones and interfaces first, then the built-in headphone jack, then the speakers.
    std::string pickRealOutput (const std::vector<OutputDevice>& devices);

    // Whether any app other than this one is sending audio to an output right now.
    bool isAnotherProcessPlaying();

    // Apps that also process all system audio, so running them alongside means hearing two EQs.
    struct RunningApp
    {
        std::string name, bundleID;
        int pid = 0;
    };
    std::vector<RunningApp> getConflictingApps();
    bool quitApp (int pid);

    // Opens System Settings at Privacy & Security > Screen & System Audio Recording.
    void openAudioCapturePrivacySettings();

    // Calls `onChange` on the main thread when devices come or go, or the default output changes.
    class ChangeListener
    {
    public:
        explicit ChangeListener (std::function<void()> onChange);
        ~ChangeListener();

    private:
        struct Impl;
        std::unique_ptr<Impl> impl;
    };
}
