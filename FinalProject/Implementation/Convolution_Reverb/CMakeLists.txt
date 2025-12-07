cmake_minimum_required(VERSION 3.15)

project(Convolution_Reverb VERSION 0.1.0)

# Point JUCE_DIR to your JUCE clone (the directory containing JUCEConfig.cmake).
# Example configure step:
#   cmake -B build -S . -DJUCE_DIR=/path/to/JUCE
if(NOT JUCE_DIR)
    message(FATAL_ERROR "Please set JUCE_DIR to the path of your JUCE checkout (folder containing JUCEConfig.cmake).")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(Convolution_Reverb
    COMPANY_NAME "ConvolutionLab"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE CvRb
    PLUGIN_CODE CvRv
    FORMATS AU VST3
    PRODUCT_NAME "Convolution_Reverb")

target_sources(Convolution_Reverb PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ConvolutionEngine.cpp
    Source/IRLoader.cpp)

target_compile_features(Convolution_Reverb PRIVATE cxx_std_17)

target_compile_definitions(Convolution_Reverb PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Convolution_Reverb PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_audio_basics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core)

juce_generate_juce_header(Convolution_Reverb)
