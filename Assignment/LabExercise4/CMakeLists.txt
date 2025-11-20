cmake_minimum_required(VERSION 3.15)

project(LabExercise4 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE path
set(JUCE_DIR "/Users/wujing/Documents/GitHub/JUCE")

add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_console_app(LabExercise4
    PRODUCT_NAME "LabExercise4"
)

target_sources(LabExercise4
    PRIVATE
        src/Main.cpp
)

target_link_libraries(LabExercise4
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_formats
)
