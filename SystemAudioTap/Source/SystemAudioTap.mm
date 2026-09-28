#include "SystemAudioTap.h"

#import <CoreAudio/AudioHardwareTapping.h>
#import <CoreAudio/CATapDescription.h>
#import <CoreAudio/CoreAudio.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <dlfcn.h>
#include <unistd.h>
#include <vector>

#if ! __has_feature(objc_arc)
 #error "SystemAudioTap.mm must be compiled with -fobjc-arc"
#endif

namespace
{
constexpr int maxFrames = 8192;

template <typename Value>
bool getProperty (AudioObjectID object, AudioObjectPropertySelector selector, Value& value,
                  AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal)
{
    const AudioObjectPropertyAddress address { selector, scope, kAudioObjectPropertyElementMain };
    UInt32 size = sizeof (Value);
    return AudioObjectGetPropertyData (object, &address, 0, nullptr, &size, &value) == noErr;
}

std::string getStringProperty (AudioObjectID object, AudioObjectPropertySelector selector)
{
    CFStringRef value = nullptr;

    if (! getProperty (object, selector, value) || value == nullptr)
        return {};

    NSString* string = CFBridgingRelease (value);
    return string.UTF8String != nullptr ? string.UTF8String : "";
}

int countChannels (AudioObjectID device, AudioObjectPropertyScope scope)
{
    const AudioObjectPropertyAddress address { kAudioDevicePropertyStreamConfiguration, scope, kAudioObjectPropertyElementMain };
    UInt32 size = 0;

    if (AudioObjectGetPropertyDataSize (device, &address, 0, nullptr, &size) != noErr || size == 0)
        return 0;

    std::vector<UInt8> storage (size);
    auto* list = reinterpret_cast<AudioBufferList*> (storage.data());

    if (AudioObjectGetPropertyData (device, &address, 0, nullptr, &size, list) != noErr)
        return 0;

    int channels = 0;

    for (UInt32 i = 0; i < list->mNumberBuffers; ++i)
        channels += (int) list->mBuffers[i].mNumberChannels;

    return channels;
}

// Core Audio errors are often four-character codes like 'who?'.
std::string statusText (OSStatus status)
{
    const char code[] = { char (status >> 24), char (status >> 16), char (status >> 8), char (status), 0 };
    const bool printable = std::all_of (code, code + 4, [] (char c) { return c >= 32 && c < 127; });
    return (printable ? "'" + std::string (code) + "' " : std::string()) + "(" + std::to_string (status) + ")";
}

std::string describeFormat (const AudioStreamBasicDescription& format)
{
    char text[128];
    snprintf (text, sizeof (text), "%.0f Hz, %u ch, %u-bit %s, %s",
              format.mSampleRate, (unsigned) format.mChannelsPerFrame, (unsigned) format.mBitsPerChannel,
              (format.mFormatFlags & kAudioFormatFlagIsFloat) ? "float" : "int",
              (format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ? "non-interleaved" : "interleaved");
    return text;
}

// Finds channel `channel` of an AudioBufferList, numbering the channels of all its buffers in order.
const float* locateChannel (const AudioBufferList* list, int channel, int& stride)
{
    if (list == nullptr)
        return nullptr;

    for (UInt32 i = 0; i < list->mNumberBuffers; ++i)
    {
        const auto& buffer = list->mBuffers[i];

        if (channel < (int) buffer.mNumberChannels)
        {
            stride = (int) buffer.mNumberChannels;
            return buffer.mData != nullptr ? static_cast<const float*> (buffer.mData) + channel : nullptr;
        }

        channel -= (int) buffer.mNumberChannels;
    }

    return nullptr;
}
}

//==============================================================================
struct SystemAudioTap::Impl
{
    CATapDescription* tapDescription = nil;
    AudioObjectID tapID = kAudioObjectUnknown;
    AudioObjectID aggregateID = kAudioObjectUnknown;
    AudioDeviceIOProcID ioProcID = nullptr;

    int tapChannels = 2;
    int firstTapChannel = 0;
    double sampleRate = 0;
    int bufferSize = 0;
    std::string description;

    ProcessFn process;
    std::vector<float> inL, inR, outL, outR;   // sized before starting, so the audio thread never allocates

    std::string outputName, outputUID;
    std::atomic<float> inPeak { 0 }, outPeak { 0 };
    std::atomic<int> callbacks { 0 };

    static void raise (std::atomic<float>& peak, const float* data, int n)
    {
        float loudest = 0;
        for (int i = 0; i < n; ++i)
            loudest = std::max (loudest, std::abs (data[i]));

        auto current = peak.load();
        while (loudest > current && ! peak.compare_exchange_weak (current, loudest)) {}
    }

    void render (const AudioBufferList* input, AudioBufferList* output)
    {
        if (output == nullptr || output->mNumberBuffers == 0 || output->mBuffers[0].mNumberChannels == 0)
            return;

        const auto& firstBuffer = output->mBuffers[0];
        const int numFrames = std::min (maxFrames, (int) (firstBuffer.mDataByteSize / (sizeof (float) * firstBuffer.mNumberChannels)));

        float* in[] = { inL.data(), inR.data() };
        float* out[] = { outL.data(), outR.data() };

        for (int ch = 0; ch < 2; ++ch)
        {
            int stride = 1;

            // A mono tap feeds both sides.
            if (const float* source = locateChannel (input, firstTapChannel + std::min (ch, tapChannels - 1), stride))
            {
                for (int i = 0; i < numFrames; ++i)
                    in[ch][i] = source[i * stride];
            }
            else
            {
                std::fill_n (in[ch], numFrames, 0.0f);
            }
        }

        process (in, out, numFrames);

        for (int ch = 0; ch < 2; ++ch)
        {
            raise (inPeak, in[ch], numFrames);
            raise (outPeak, out[ch], numFrames);
        }
        callbacks.fetch_add (1);

        // The stereo result goes to the first two output channels, silence to any others.
        int channel = 0;

        for (UInt32 b = 0; b < output->mNumberBuffers; ++b)
        {
            auto& buffer = output->mBuffers[b];
            auto* dest = static_cast<float*> (buffer.mData);
            const int stride = (int) buffer.mNumberChannels;

            for (int c = 0; c < stride; ++c, ++channel)
                if (dest != nullptr)
                    for (int i = 0; i < numFrames; ++i)
                        dest[i * stride + c] = channel < 2 ? out[channel][i] : 0.0f;
        }
    }
};

//==============================================================================
SystemAudioTap::SystemAudioTap() : impl (std::make_unique<Impl>()) {}

SystemAudioTap::~SystemAudioTap()
{
    close();
}

namespace
{
using PreflightFn = long (*) (CFStringRef, CFDictionaryRef);
using RequestFn = void (*) (CFStringRef, CFDictionaryRef, void (^) (BOOL));

// There's no public API to ask about this permission up front (without it, the tap just
// delivers silence), so this uses the private TCC calls, as the AudioCap sample does.
void* openTCC()
{
    static void* tcc = dlopen ("/System/Library/PrivateFrameworks/TCC.framework/Versions/A/TCC", RTLD_NOW);
    return tcc;
}
}

SystemAudioTap::Permission SystemAudioTap::checkPermission()
{
    auto* tcc = openTCC();
    const auto preflight = tcc != nullptr ? reinterpret_cast<PreflightFn> (dlsym (tcc, "TCCAccessPreflight")) : nullptr;

    if (preflight == nullptr)
        return Permission::unknown;

    switch (preflight (CFSTR ("kTCCServiceAudioCapture"), nullptr))
    {
        case 0:  return Permission::granted;
        case 1:  return Permission::denied;
        default: return Permission::unknown;
    }
}

SystemAudioTap::Permission SystemAudioTap::requestPermission()
{
    auto* tcc = openTCC();

    if (tcc == nullptr)
        return Permission::unknown;

    const auto preflight = reinterpret_cast<PreflightFn> (dlsym (tcc, "TCCAccessPreflight"));
    const auto request = reinterpret_cast<RequestFn> (dlsym (tcc, "TCCAccessRequest"));

    if (preflight == nullptr || request == nullptr)
        return Permission::unknown;

    const auto service = CFSTR ("kTCCServiceAudioCapture");

    switch (preflight (service, nullptr))
    {
        case 0:  return Permission::granted;
        case 1:  return Permission::denied;
        default: break;
    }

    __block BOOL granted = NO;
    dispatch_semaphore_t answered = dispatch_semaphore_create (0);

    request (service, nullptr, ^(BOOL result) {
        granted = result;
        dispatch_semaphore_signal (answered);
    });

    if (dispatch_semaphore_wait (answered, dispatch_time (DISPATCH_TIME_NOW, 120 * NSEC_PER_SEC)) != 0)
        return Permission::unknown;

    return granted ? Permission::granted : Permission::denied;
}

bool SystemAudioTap::open (std::string& error, const std::string& requestedOutputUID)
{
    @autoreleasepool
    {
        close();
        auto& s = *impl;

        // 1. Find this process in Core Audio so the tap can leave it out. Otherwise the
        //    processed audio this app plays would be captured again: endless feedback.
        AudioObjectID thisProcess = kAudioObjectUnknown;
        const AudioObjectPropertyAddress translate { kAudioHardwarePropertyTranslatePIDToProcessObject,
                                                     kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
        const pid_t pid = getpid();
        UInt32 size = sizeof (thisProcess);
        AudioObjectGetPropertyData (kAudioObjectSystemObject, &translate, sizeof (pid), &pid, &size, &thisProcess);

        if (thisProcess == kAudioObjectUnknown)
        {
            error = "couldn't find this process in Core Audio, so it can't be left out of the tap";
            return false;
        }

        // 2. A stereo mix of every other process's output. While something reads the tap,
        //    those processes are muted, so only the processed version is heard. If this app
        //    quits or crashes, nothing's reading it any more and normal audio comes back.
        s.tapDescription = [[CATapDescription alloc] initStereoGlobalTapButExcludeProcesses:@[ @(thisProcess) ]];
        s.tapDescription.name = @"CabinEQ System";
        s.tapDescription.privateTap = YES;
        s.tapDescription.muteBehavior = CATapMutedWhenTapped;

        if (auto status = AudioHardwareCreateProcessTap (s.tapDescription, &s.tapID); status != noErr)
        {
            error = "AudioHardwareCreateProcessTap failed: " + statusText (status);
            return false;
        }

        AudioStreamBasicDescription tapFormat {};
        getProperty (s.tapID, kAudioTapPropertyFormat, tapFormat);
        s.tapChannels = std::max (1, (int) tapFormat.mChannelsPerFrame);

        // 3. A private aggregate device (only this process can see it) that runs the tap and
        //    the current output device on one clock: tap in, speakers out.
        AudioObjectID outputDevice = kAudioObjectUnknown;
        getProperty (kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, outputDevice);
        auto outputUID = getStringProperty (outputDevice, kAudioDevicePropertyDeviceUID);

        if (! requestedOutputUID.empty() && requestedOutputUID != outputUID)
        {
            outputUID = requestedOutputUID;
            outputDevice = kAudioObjectUnknown;

            const AudioObjectPropertyAddress devices { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
            UInt32 devicesSize = 0;
            AudioObjectGetPropertyDataSize (kAudioObjectSystemObject, &devices, 0, nullptr, &devicesSize);
            std::vector<AudioObjectID> ids (devicesSize / sizeof (AudioObjectID));
            AudioObjectGetPropertyData (kAudioObjectSystemObject, &devices, 0, nullptr, &devicesSize, ids.data());

            for (auto id : ids)
                if (getStringProperty (id, kAudioDevicePropertyDeviceUID) == outputUID)
                    outputDevice = id;
        }

        if (outputUID.empty() || outputDevice == kAudioObjectUnknown)
        {
            error = "couldn't find the output device";
            return false;
        }

        s.outputUID = outputUID;
        s.outputName = getStringProperty (outputDevice, kAudioObjectPropertyName);

        NSString* outputUIDString = @(outputUID.c_str());
        NSDictionary* aggregate = @{
            @kAudioAggregateDeviceNameKey:          @"CabinEQ System",
            @kAudioAggregateDeviceUIDKey:           NSUUID.UUID.UUIDString,
            @kAudioAggregateDeviceIsPrivateKey:     @YES,
            @kAudioAggregateDeviceMainSubDeviceKey: outputUIDString,
            @kAudioAggregateDeviceSubDeviceListKey: @[ @{ @kAudioSubDeviceUIDKey: outputUIDString } ],
            @kAudioAggregateDeviceTapListKey:       @[ @{ @kAudioSubTapUIDKey: s.tapDescription.UUID.UUIDString,
                                                          @kAudioSubTapDriftCompensationKey: @YES } ],
            @kAudioAggregateDeviceTapAutoStartKey:  @YES,
        };

        if (auto status = AudioHardwareCreateAggregateDevice ((__bridge CFDictionaryRef) aggregate, &s.aggregateID); status != noErr)
        {
            error = "AudioHardwareCreateAggregateDevice failed: " + statusText (status);
            return false;
        }

        // The tap's channels come after the output device's own inputs, if it has any
        // (an audio interface would, built-in speakers don't).
        int aggregateInputs = 0;

        for (int attempt = 0; attempt < 40 && aggregateInputs < s.tapChannels; ++attempt)
        {
            aggregateInputs = countChannels (s.aggregateID, kAudioObjectPropertyScopeInput);

            if (aggregateInputs < s.tapChannels)
                usleep (50'000);
        }

        if (aggregateInputs < s.tapChannels)
        {
            error = "the aggregate device never showed the tap's input channels";
            return false;
        }

        s.firstTapChannel = aggregateInputs - s.tapChannels;

        Float64 sampleRate = 0;
        UInt32 bufferSize = 0;
        getProperty (s.aggregateID, kAudioDevicePropertyNominalSampleRate, sampleRate);
        getProperty (s.aggregateID, kAudioDevicePropertyBufferFrameSize, bufferSize);
        s.sampleRate = sampleRate;
        s.bufferSize = (int) bufferSize;

        s.description = "Output device: " + getStringProperty (outputDevice, kAudioObjectPropertyName)
                      + "\nTap format: " + describeFormat (tapFormat)
                      + "\nAggregate device: " + std::to_string ((int) sampleRate) + " Hz, "
                      + std::to_string (bufferSize) + "-frame buffer, "
                      + std::to_string (aggregateInputs) + " input ch (tap from ch " + std::to_string (s.firstTapChannel) + "), "
                      + std::to_string (countChannels (s.aggregateID, kAudioObjectPropertyScopeOutput)) + " output ch";
        return true;
    }
}

bool SystemAudioTap::start (ProcessFn process, std::string& error)
{
    auto& s = *impl;

    if (s.aggregateID == kAudioObjectUnknown)
    {
        error = "the tap isn't open";
        return false;
    }

    stop();
    s.process = std::move (process);

    for (auto* scratch : { &s.inL, &s.inR, &s.outL, &s.outR })
        scratch->assign (maxFrames, 0.0f);

    Impl* state = &s;
    auto status = AudioDeviceCreateIOProcIDWithBlock (&s.ioProcID, s.aggregateID, nullptr,
                                                      ^(const AudioTimeStamp*, const AudioBufferList* input, const AudioTimeStamp*,
                                                        AudioBufferList* output, const AudioTimeStamp*) {
                                                          state->render (input, output);
                                                      });

    if (status == noErr)
        status = AudioDeviceStart (s.aggregateID, s.ioProcID);

    if (status != noErr)
    {
        error = "couldn't start audio on the aggregate device: " + statusText (status);
        stop();
        return false;
    }

    return true;
}

void SystemAudioTap::stop()
{
    auto& s = *impl;

    if (s.ioProcID != nullptr)
    {
        AudioDeviceStop (s.aggregateID, s.ioProcID);
        AudioDeviceDestroyIOProcID (s.aggregateID, s.ioProcID);
        s.ioProcID = nullptr;
    }
}

void SystemAudioTap::close()
{
    stop();
    auto& s = *impl;

    if (s.aggregateID != kAudioObjectUnknown)
    {
        AudioHardwareDestroyAggregateDevice (s.aggregateID);
        s.aggregateID = kAudioObjectUnknown;
    }

    if (s.tapID != kAudioObjectUnknown)
    {
        AudioHardwareDestroyProcessTap (s.tapID);
        s.tapID = kAudioObjectUnknown;
    }

    s.tapDescription = nil;
}

std::string SystemAudioTap::getOutputName() const  { return impl->outputName; }
std::string SystemAudioTap::getOutputUID() const   { return impl->outputUID; }
bool SystemAudioTap::isRunning() const              { return impl->ioProcID != nullptr; }

SystemAudioTap::Activity SystemAudioTap::takeActivity()
{
    return { impl->inPeak.exchange (0.0f), impl->outPeak.exchange (0.0f), impl->callbacks.exchange (0) };
}

double SystemAudioTap::getSampleRate() const       { return impl->sampleRate; }
int SystemAudioTap::getBufferSize() const          { return impl->bufferSize; }
std::string SystemAudioTap::getDescription() const { return impl->description; }
