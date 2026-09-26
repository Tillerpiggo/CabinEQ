cmake_minimum_required(VERSION 3.22)

# Process taps need macOS 14.2+
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.2" CACHE STRING "")

project(TapTest VERSION 0.1.0 LANGUAGES C CXX)

set(JUCE_DIR "$ENV{HOME}/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_gui_app(TapTest
    PRODUCT_NAME "TapTest"
    BUNDLE_ID "com.julianmcomie.taptest"
    # Without this usage string macOS never asks for the permission, and the tap just delivers silence.
    PLIST_TO_MERGE [[
        <plist><dict>
            <key>NSAudioCaptureUsageDescription</key>
            <string>TapTest routes all system audio through your plugin.</string>
        </dict></plist>
    ]])

target_sources(TapTest PRIVATE
    Source/Main.cpp
    Source/SystemAudioTap.mm)

set_source_files_properties(Source/SystemAudioTap.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")

target_compile_definitions(TapTest PRIVATE
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

target_link_libraries(TapTest
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_dsp
        "-framework CoreAudio"
        "-framework CoreAudioKit"
        "-framework Foundation"
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
