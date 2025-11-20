cmake_minimum_required(VERSION 3.15)

project(JUCE_Plugin VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to your local JUCE clone
# You can change this in CMake GUI / ccmake later if needed.
set(JUCE_DIR "/Users/wujing/Documents/GitHub/JUCE" CACHE PATH "Path to JUCE")

# Add JUCE to the build
add_subdirectory(${JUCE_DIR} JUCE)

# ====================================================================
# Plugin target
# ====================================================================
juce_add_plugin(JUCE_Plugin
    COMPANY_NAME              "Kevin Ren"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   TRUE

    PLUGIN_MANUFACTURER_CODE  JRen
    PLUGIN_CODE               MyPl

    FORMATS                   VST3
    PRODUCT_NAME              "JUCE_Plugin"
)

target_sources(JUCE_Plugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
)

target_link_libraries(JUCE_Plugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
)
