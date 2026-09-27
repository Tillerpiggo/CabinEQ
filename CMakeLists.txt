cmake_minimum_required(VERSION 3.22)

# A CMake build of CabinEQ that mirrors CabinEQ.jucer, so the plugin builds with just the
# Command Line Tools (no Xcode). update.sh uses it to build and install the VST3.
project(CabinEQ VERSION 1.0.0 LANGUAGES C CXX)

set(JUCE_DIR "$ENV{HOME}/JUCE" CACHE PATH "Path to JUCE (8.0.4 or newer, for the macOS 15+ SDK)")
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "Couldn't find JUCE at ${JUCE_DIR}.\n"
        "Clone it with: git clone --depth 1 --branch 8.0.15 https://github.com/juce-framework/JUCE.git ~/JUCE\n"
        "or pass -DJUCE_DIR=<your JUCE folder>.")
endif()
add_subdirectory(${JUCE_DIR} JUCE)

# The icons and font the UI loads.
set(CABINEQ_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources" CACHE PATH "Folder with CabinEQ's icons and font")

set(resourceNames
    CabinEQIconEighty.png
    EllipsisIcon.png
    Inter.ttf
    MuteIcon.png
    NextButtonIcon.png
    PauseButtonIcon.png
    PlayButtonIcon.png
    PlusIcon.png
    PrevButtonIcon.png
    QuestionMarkIcon.png
    SpeakerIcon.png)

set(resourceFiles)
set(missingResources)

foreach(name IN LISTS resourceNames)
    if(EXISTS "${CABINEQ_RESOURCES_DIR}/${name}")
        list(APPEND resourceFiles "${CABINEQ_RESOURCES_DIR}/${name}")
    else()
        list(APPEND missingResources "${name}")
    endif()
endforeach()

if(missingResources)
    list(JOIN missingResources "\n    " missingText)
    message(FATAL_ERROR
        "CabinEQ's UI needs these files in ${CABINEQ_RESOURCES_DIR}:\n    ${missingText}\n"
        "Add them there, or pass -DCABINEQ_RESOURCES_DIR=<a folder that has them>.")
endif()

juce_add_binary_data(CabinEQ_BinaryData SOURCES ${resourceFiles})

# Same identity as the Projucer build (manufacturer 'Manu', plugin 'Tuqj'), so a build from
# here replaces the installed plugin in place.
juce_add_plugin(CabinEQ
    PRODUCT_NAME "CabinEQ"
    COMPANY_NAME "yourcompany"
    COMPANY_WEBSITE "www.yourcompany.com"
    BUNDLE_ID "com.yourcompany.CabinEQ"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Tuqj
    FORMATS VST3 Standalone
    ICON_BIG "${CABINEQ_RESOURCES_DIR}/CabinEQIconEighty.png"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This app requires microphone input to route system audio."
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(CabinEQ)

target_sources(CabinEQ PRIVATE
    Source/AddProfileButton.cpp
    Source/AnimatedGlyph.cpp
    Source/ArbitraryResponseFilter.cpp
    Source/ArchetypalGlyph.cpp
    Source/ArchetypeBar.cpp
    Source/ArchetypeView.cpp
    Source/BandEqCurve.cpp
    Source/BandProfile.cpp
    Source/BuildableComponent.cpp
    Source/CabinEqAudioProcessor.cpp
    Source/CabinEqPage.cpp
    Source/CabinEqProcessorEditor.cpp
    Source/CabinEqProfile.cpp
    Source/CabinEqProfileManager.cpp
    Source/CabinPeqGraph.cpp
    Source/CabinStandalone.cpp
    Source/CalibrationView.cpp
    Source/Checkerboard.cpp
    Source/CheckerboardManager.cpp
    Source/CheckerboardPlayer.cpp
    Source/CheckerboardView.cpp
    Source/ContactUsBanner.cpp
    Source/CrossfeedControl.cpp
    Source/CrossfeedProcessor.cpp
    Source/Curve.cpp
    Source/ElevationCalibration.cpp
    Source/ElevationCalibrationManager.cpp
    Source/ElevationCalibrationPlayer.cpp
    Source/FreeTrialBanner.cpp
    Source/FreeTrialLockScreen.cpp
    Source/GainEnvelope.cpp
    Source/Glyph.cpp
    Source/GlyphGenerator.cpp
    Source/GlyphGridPlayer.cpp
    Source/GlyphGridView.cpp
    Source/GlyphManager.cpp
    Source/GlyphView.cpp
    Source/Layout.cpp
    Source/MusicList2.cpp
    Source/NoiseGenerator.cpp
    Source/NoiseSource.cpp
    Source/PatternEnvelope.cpp
    Source/PlaybackManager.cpp
    Source/ProfileList.cpp
    Source/ProfileRow.cpp
    Source/ProfileView.cpp
    Source/Row.cpp
    Source/RowPlayer.cpp
    Source/SquareGenerator.cpp
    Source/Stroke.cpp
    Source/TutorialButton.cpp)

# The jucer's project defines and JUCE module options
target_compile_definitions(CabinEQ PUBLIC
    DONT_SET_USING_JUCE_NAMESPACE=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_USE_LAME_AUDIO_FORMAT=1
    JUCE_USE_WINDOWS_MEDIA_FORMAT=1)

# The jucer's Release configuration uses fast math
target_compile_options(CabinEQ PRIVATE $<$<CONFIG:Release>:-ffast-math>)

target_link_libraries(CabinEQ
    PRIVATE
        CabinEQ_BinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_cryptography
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_opengl
        juce::juce_product_unlocking
    PUBLIC
        juce::juce_recommended_config_flags)
