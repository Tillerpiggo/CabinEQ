#include "AudioDevices.h"

#import <AppKit/AppKit.h>
#import <CoreAudio/CoreAudio.h>
#import <Foundation/Foundation.h>

#include <algorithm>
#include <unistd.h>

#if ! __has_feature(objc_arc)
 #error "AudioDevices.mm must be compiled with -fobjc-arc"
#endif

namespace
{
template <typename Value>
bool getProperty (AudioObjectID object, AudioObjectPropertySelector selector, Value& value,
                  AudioObjectPropertyScope scope = kAudioObjectPropertyScopeGlobal)
{
    const AudioObjectPropertyAddress address { selector, scope, kAudioObjectPropertyElementMain };
    UInt32 size = sizeof (Value);
    return AudioObjectGetPropertyData (object, &address, 0, nullptr, &size, &value) == noErr;
}

template <typename Value>
std::vector<Value> getArrayProperty (AudioObjectID object, AudioObjectPropertySelector selector)
{
    const AudioObjectPropertyAddress address { selector, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
    UInt32 size = 0;

    if (AudioObjectGetPropertyDataSize (object, &address, 0, nullptr, &size) != noErr || size == 0)
        return {};

    std::vector<Value> values (size / sizeof (Value));

    if (AudioObjectGetPropertyData (object, &address, 0, nullptr, &size, values.data()) != noErr)
        return {};

    values.resize (size / sizeof (Value));
    return values;
}

std::string getStringProperty (AudioObjectID object, AudioObjectPropertySelector selector)
{
    CFStringRef value = nullptr;

    if (! getProperty (object, selector, value) || value == nullptr)
        return {};

    NSString* string = CFBridgingRelease (value);
    return string.UTF8String != nullptr ? string.UTF8String : "";
}

int countOutputChannels (AudioObjectID device)
{
    const AudioObjectPropertyAddress address { kAudioDevicePropertyStreamConfiguration, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMain };
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

AudioObjectID deviceWithUID (const std::string& uid)
{
    for (auto device : getArrayProperty<AudioObjectID> (kAudioObjectSystemObject, kAudioHardwarePropertyDevices))
        if (getStringProperty (device, kAudioDevicePropertyDeviceUID) == uid)
            return device;

    return kAudioObjectUnknown;
}

bool containsAny (const std::string& text, std::initializer_list<const char*> words)
{
    NSString* haystack = @(text.c_str());

    for (auto* word : words)
        if ([haystack rangeOfString:@(word) options:NSCaseInsensitiveSearch].location != NSNotFound)
            return true;

    return false;
}
}

namespace AudioDevices
{
std::vector<OutputDevice> getOutputDevices()
{
    std::vector<OutputDevice> result;

    for (auto device : getArrayProperty<AudioObjectID> (kAudioObjectSystemObject, kAudioHardwarePropertyDevices))
    {
        if (countOutputChannels (device) == 0)
            continue;

        UInt32 transport = 0;
        getProperty (device, kAudioDevicePropertyTransportType, transport);

        if (transport == kAudioDeviceTransportTypeAggregate || transport == kAudioDeviceTransportTypeAutoAggregate)
            continue;

        OutputDevice info;
        info.uid = getStringProperty (device, kAudioDevicePropertyDeviceUID);
        info.name = getStringProperty (device, kAudioObjectPropertyName);
        info.isBuiltIn = transport == kAudioDeviceTransportTypeBuiltIn;
        info.isBluetooth = transport == kAudioDeviceTransportTypeBluetooth || transport == kAudioDeviceTransportTypeBluetoothLE;

        // Virtual drivers mostly say so, but some (eqMac) claim to be USB, so also go by name
        info.isVirtual = transport == kAudioDeviceTransportTypeVirtual
                      || transport == kAudioDeviceTransportTypeUnknown
                      || containsAny (info.name, { "BlackHole", "Soundflower", "Loopback", "eqMac", "Boom", "Dolby Audio Bridge",
                                                   "ZoomAudio", "Microsoft Teams", "SRAudio", "Background Music", "VB-Cable",
                                                   "Rogue Amoeba", "CabinEQ" });

        if (! info.uid.empty())
            result.push_back (info);
    }

    return result;
}

std::string getDefaultOutputUID()
{
    AudioObjectID device = kAudioObjectUnknown;
    getProperty (kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, device);
    return device != kAudioObjectUnknown ? getStringProperty (device, kAudioDevicePropertyDeviceUID) : std::string();
}

bool setDefaultOutput (const std::string& uid)
{
    AudioObjectID device = deviceWithUID (uid);

    if (device == kAudioObjectUnknown)
        return false;

    bool ok = true;

    for (auto selector : { kAudioHardwarePropertyDefaultOutputDevice, kAudioHardwarePropertyDefaultSystemOutputDevice })
    {
        const AudioObjectPropertyAddress address { selector, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
        ok = AudioObjectSetPropertyData (kAudioObjectSystemObject, &address, 0, nullptr, sizeof (device), &device) == noErr && ok;
    }

    return ok;
}

std::string pickRealOutput (const std::vector<OutputDevice>& devices)
{
    auto score = [] (const OutputDevice& d)
    {
        if (d.isVirtual)                                               return 0;
        if (! d.isBuiltIn)                                             return 4;   // Bluetooth, USB, interfaces
        if (containsAny (d.name, { "Headphone" }))                     return 3;
        if (containsAny (d.name, { "Speaker" }))                       return 2;
        return 1;
    };

    const OutputDevice* best = nullptr;

    for (const auto& device : devices)
        if (score (device) > 0 && (best == nullptr || score (device) > score (*best)))
            best = &device;

    return best != nullptr ? best->uid : std::string();
}

bool isAnotherProcessPlaying()
{
    const pid_t thisProcess = getpid();

    for (auto process : getArrayProperty<AudioObjectID> (kAudioObjectSystemObject, kAudioHardwarePropertyProcessObjectList))
    {
        pid_t pid = 0;
        UInt32 isRunningOutput = 0;

        if (getProperty (process, kAudioProcessPropertyPID, pid) && pid != thisProcess
            && getProperty (process, kAudioProcessPropertyIsRunningOutput, isRunningOutput) && isRunningOutput != 0)
            return true;
    }

    return false;
}

std::vector<RunningApp> getConflictingApps()
{
    static const std::pair<const char*, const char*> known[] {
        { "com.bitgapp.eqmac",           "eqMac" },
        { "com.globaldelight.Boom3D",    "Boom 3D" },
        { "com.globaldelight.Boom2",     "Boom 2" },
        { "com.rogueamoeba.soundsource", "SoundSource" },
        { "com.yourcompany.CabinEQ",     "CabinEQ (the standalone app)" },
    };

    std::vector<RunningApp> result;

    for (NSRunningApplication* app in NSWorkspace.sharedWorkspace.runningApplications)
    {
        if (app.bundleIdentifier == nil || app.processIdentifier == getpid())
            continue;

        for (const auto& [bundleID, name] : known)
            if ([app.bundleIdentifier isEqualToString:@(bundleID)])
                result.push_back ({ name, bundleID, (int) app.processIdentifier });
    }

    return result;
}

bool quitApp (int pid)
{
    NSRunningApplication* app = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
    return app != nil && [app terminate];
}

void openAudioCapturePrivacySettings()
{
    [NSWorkspace.sharedWorkspace openURL:[NSURL URLWithString:@"x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture"]];
}

//==============================================================================
struct ChangeListener::Impl
{
    std::function<void()> onChange;
    AudioObjectPropertyListenerBlock block = nil;
    std::vector<AudioObjectPropertyAddress> addresses;
};

ChangeListener::ChangeListener (std::function<void()> onChange) : impl (std::make_unique<Impl>())
{
    impl->onChange = std::move (onChange);
    auto* state = impl.get();
    impl->block = ^(UInt32, const AudioObjectPropertyAddress*) { state->onChange(); };

    for (auto selector : { kAudioHardwarePropertyDevices, kAudioHardwarePropertyDefaultOutputDevice })
    {
        const AudioObjectPropertyAddress address { selector, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain };
        if (AudioObjectAddPropertyListenerBlock (kAudioObjectSystemObject, &address, dispatch_get_main_queue(), impl->block) == noErr)
            impl->addresses.push_back (address);
    }
}

ChangeListener::~ChangeListener()
{
    for (const auto& address : impl->addresses)
        AudioObjectRemovePropertyListenerBlock (kAudioObjectSystemObject, &address, dispatch_get_main_queue(), impl->block);
}
}
