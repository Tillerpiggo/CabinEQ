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
    Inter.ttf)

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
    Source/BandEqCurve.cpp
    Source/BandInspector.cpp
    Source/BandProfile.cpp
    Source/CabinEqAudioProcessor.cpp
    Source/CabinEqLookAndFeel.cpp
    Source/CabinEqPage.cpp
    Source/CabinEqProcessorEditor.cpp
    Source/CabinEqProfile.cpp
    Source/CabinEqProfileManager.cpp
    Source/CabinPeqGraph.cpp
    Source/CabinStandalone.cpp
    Source/CrossfeedControl.cpp
    Source/CrossfeedProcessor.cpp
    Source/EqPresetFile.cpp
    Source/PlaybackManager.cpp
    Source/ProfileList.cpp
    Source/ValueField.cpp)

# The jucer's project defines and JUCE module options
target_compile_definitions(CabinEQ PUBLIC
    DONT_SET_USING_JUCE_NAMESPACE=1
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

# The jucer's Release configuration uses fast math
target_compile_options(CabinEQ PRIVATE $<$<CONFIG:Release>:-ffast-math>)

target_link_libraries(CabinEQ
    PRIVATE
        CabinEQ_BinaryData
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags)

# Headless tests for the DSP and saved state. They link against the plugin's shared code.
#   cmake --build build --target CabinEQ_Tests && build/CabinEQ_Tests_artefacts/Release/CabinEQ_Tests
juce_add_console_app(CabinEQ_Tests PRODUCT_NAME "CabinEQ_Tests")
target_sources(CabinEQ_Tests PRIVATE Tests/CabinEqTests.cpp)
# Compile against the same headers and settings as the plugin, without building JUCE a second time
target_include_directories(CabinEQ_Tests PRIVATE $<TARGET_PROPERTY:CabinEQ,INCLUDE_DIRECTORIES>)
target_compile_definitions(CabinEQ_Tests PRIVATE $<TARGET_PROPERTY:CabinEQ,COMPILE_DEFINITIONS>)
target_compile_features(CabinEQ_Tests PRIVATE cxx_std_17)
add_dependencies(CabinEQ_Tests CabinEQ)
target_compile_definitions(CabinEQ_Tests PRIVATE JUCE_UNIT_TESTS=1)
set_target_properties(CabinEQ_Tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(CabinEQ_Tests PRIVATE CabinEQ juce::juce_recommended_config_flags)

# Loads the built VST3 the way a DAW would, as a stand-in for pluginval.
#   cmake --build build --target CabinEQ_HostCheck
#   build/CabinEQ_HostCheck_artefacts/Release/CabinEQ_HostCheck build/CabinEQ_artefacts/Release/VST3/CabinEQ.vst3
juce_add_console_app(CabinEQ_HostCheck PRODUCT_NAME "CabinEQ_HostCheck")
target_sources(CabinEQ_HostCheck PRIVATE Tests/HostCheck.cpp)
target_compile_definitions(CabinEQ_HostCheck PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
set_target_properties(CabinEQ_HostCheck PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(CabinEQ_HostCheck PRIVATE
    juce::juce_audio_processors juce::juce_gui_basics juce::juce_recommended_config_flags)
