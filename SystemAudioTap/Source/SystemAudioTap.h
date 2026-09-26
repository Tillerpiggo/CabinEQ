#pragma once

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

    // Asks for the "System Audio Recording" permission, showing the system prompt if the
    // user hasn't answered it yet. Blocks until they do.
    static Permission requestPermission();

    SystemAudioTap();
    ~SystemAudioTap();

    // Creates the tap, plus a private aggregate device with the tap as its input and the
    // current default output device as its output.
    bool open (std::string& error);
    bool start (ProcessFn process, std::string& error);
    void stop();
    void close();

    double getSampleRate() const;
    int getBufferSize() const;
    std::string getDescription() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
