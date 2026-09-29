cmake_minimum_required(VERSION 3.22)

# Process taps need macOS 14.2+
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.2" CACHE STRING "")

project(CabinEQSystem VERSION 1.2.0 LANGUAGES C CXX)

set(JUCE_DIR "$ENV{HOME}/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_gui_app(CabinEQSystem
    PRODUCT_NAME "CabinEQ System"
    COMPANY_NAME "Cabin Audio"
    COMPANY_WEBSITE "https://cabinaudio.com"
    COMPANY_COPYRIGHT "Copyright 2026 Cabin Audio"
    BUNDLE_ID "com.julianmcomie.cabineqsystem"
    # Resources/make-icon.py draws this
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/CabinEQSystemIcon.png"
    # Without this usage string macOS never asks for the permission, and the tap just delivers silence.
    PLIST_TO_MERGE [[
        <plist><dict>
            <key>LSMinimumSystemVersion</key>
            <string>14.2</string>
            <key>NSAudioCaptureUsageDescription</key>
            <string>CabinEQ System runs all system audio through CabinEQ.</string>
        </dict></plist>
    ]])

target_sources(CabinEQSystem PRIVATE
    Source/AppView.cpp
    Source/AudioDevices.mm
    Source/Engine.cpp
    Source/Main.cpp
    Source/SystemAudioTap.mm)

set_source_files_properties(Source/SystemAudioTap.mm Source/AudioDevices.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")

# Sign the whole bundle (the linker only signs the binary), so macOS remembers the System Audio
# Recording permission. A real identity keeps it across rebuilds; an ad hoc signature ("-")
# changes every build, so macOS asks again. Uses your Apple Development identity if there is one.
if(NOT DEFINED CABINEQ_SYSTEM_SIGNING_IDENTITY)
    execute_process(COMMAND security find-identity -v -p codesigning
                    OUTPUT_VARIABLE identities ERROR_QUIET)
    string(REGEX MATCH "\"Apple Development: [^\"]+\"" identity "${identities}")
    string(REPLACE "\"" "" identity "${identity}")
    if(identity STREQUAL "")
        set(identity "-")
    endif()
    set(CABINEQ_SYSTEM_SIGNING_IDENTITY "${identity}" CACHE STRING "Code signing identity for CabinEQ System (- for ad hoc)")
endif()
message(STATUS "Signing CabinEQ System with: ${CABINEQ_SYSTEM_SIGNING_IDENTITY}")

add_custom_command(TARGET CabinEQSystem POST_BUILD
    COMMAND codesign --force --timestamp=none --sign "${CABINEQ_SYSTEM_SIGNING_IDENTITY}" "$<TARGET_BUNDLE_DIR:CabinEQSystem>"
    VERBATIM)

target_compile_definitions(CabinEQSystem PRIVATE
    CABINEQ_SYSTEM_VERSION="${PROJECT_VERSION}"
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_AU=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

target_link_libraries(CabinEQSystem
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_dsp
        "-framework CoreAudio"
        "-framework CoreAudioKit"
        "-framework Foundation"
        "-framework AppKit"
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
