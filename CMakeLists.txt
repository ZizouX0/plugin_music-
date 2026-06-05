cmake_minimum_required(VERSION 3.22)

project(Decapitone VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# macOS: build natively for Apple Silicon by default. Override with
#   -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"   for a Universal binary.
# ---------------------------------------------------------------------------
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS" FORCE)
endif()

# ---------------------------------------------------------------------------
# Pull in JUCE.  We fetch a pinned release so builds are reproducible.
# Set DECAPITONE_JUCE_PATH to a local JUCE checkout to skip the download.
# ---------------------------------------------------------------------------
if(DEFINED DECAPITONE_JUCE_PATH)
    add_subdirectory(${DECAPITONE_JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.8
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(Decapitone
    COMPANY_NAME            "ZizouAudio"
    PLUGIN_MANUFACTURER_CODE Zizu
    PLUGIN_CODE             Dcp1
    FORMATS                 AU VST3 Standalone
    PRODUCT_NAME            "Decapitone"
    BUNDLE_ID               "com.zizouaudio.decapitone"
    AU_MAIN_TYPE            kAudioUnitType_Effect
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES         Fx Distortion)

juce_generate_juce_header(Decapitone)

target_sources(Decapitone PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Decapitone PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Decapitone PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Optional headless CPU benchmark:  cmake -B build -DBUILD_BENCH=ON
# ---------------------------------------------------------------------------
option(BUILD_BENCH "Build the headless CPU benchmark" OFF)
if(BUILD_BENCH)
    juce_add_console_app(DecapitoneBench PRODUCT_NAME "DecapitoneBench")
    juce_generate_juce_header(DecapitoneBench)
    target_sources(DecapitoneBench PRIVATE
        bench/bench_main.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)
    target_compile_definitions(DecapitoneBench PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_Name="Decapitone")
    target_link_libraries(DecapitoneBench PRIVATE
        juce::juce_audio_utils juce::juce_dsp
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)

    # Objective audio-quality probe (DC offset + aliasing).
    juce_add_console_app(DecapitoneAnalyze PRODUCT_NAME "DecapitoneAnalyze")
    juce_generate_juce_header(DecapitoneAnalyze)
    target_sources(DecapitoneAnalyze PRIVATE
        bench/analyze_main.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
    target_compile_definitions(DecapitoneAnalyze PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STANDALONE_APPLICATION=1 JucePlugin_Name="Decapitone")
    target_link_libraries(DecapitoneAnalyze PRIVATE
        juce::juce_audio_utils juce::juce_dsp
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)

    # Headless UI screenshot renderer.
    juce_add_console_app(DecapitoneShot PRODUCT_NAME "DecapitoneShot")
    juce_generate_juce_header(DecapitoneShot)
    target_sources(DecapitoneShot PRIVATE
        bench/screenshot_main.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)
    target_compile_definitions(DecapitoneShot PUBLIC
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_Name="Decapitone")
    target_link_libraries(DecapitoneShot PRIVATE
        juce::juce_audio_utils juce::juce_dsp
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
endif()
