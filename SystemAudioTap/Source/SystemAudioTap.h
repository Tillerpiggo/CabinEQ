#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

// Captures the mix of every app's audio output (except this process) with a Core Audio
// process tap (macOS 14.2+) and hands it to a callback. Whatever the callback writes is
// what reaches the speakers: the original audio is muted while the tap is being read.
// No virtual audio driver is involved; the tap and its aggregate device only exist
// inside this process, and disappear when it quits.
class SystemAudioTap
{
public:
    // Runs on Core Audio's real-time thread. `in` is the tapped stereo audio, `out` is
    // where the stereo audio for the speakers goes. Both are non-interleaved.
    using ProcessFn = std::function<void (const float* const* in, float* const* out, int numFrames)>;

    enum class Permission { granted, denied, unknown };

    // Whether the "System Audio Recording" permission has been given, without asking.
    // `unknown` means it hasn't been asked yet (or macOS won't say).
    static Permission checkPermission();

    // Asks for the permission, showing the system prompt if the user hasn't answered it yet.
    // Blocks until they do, so call it off the main thread.
    static Permission requestPermission();

    SystemAudioTap();
    ~SystemAudioTap();

    // Creates the tap, plus a private aggregate device with the tap as its input and the
    // given output device (or the default one, if it's empty) as its output.
    bool open (std::string& error, const std::string& outputUID = {});
    bool start (ProcessFn process, std::string& error);
    void stop();
    void close();

    double getSampleRate() const;
    int getBufferSize() const;
    std::string getDescription() const;
    std::string getOutputName() const;
    std::string getOutputUID() const;
    bool isRunning() const;

    // What's gone through since the last call: the loudest sample in and out, and how many
    // audio callbacks ran. Safe from any thread.
    struct Activity { float inPeak = 0, outPeak = 0; int callbacks = 0; };
    Activity takeActivity();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
