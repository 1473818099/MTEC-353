cmake_minimum_required(VERSION 3.22)
project(LabExercise3 VERSION 0.1 LANGUAGES CXX)

# --- C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_DIR "${CMAKE_SOURCE_DIR}/../JUCE" CACHE PATH "Path to JUCE")

# --- Add JUCE to this build
add_subdirectory(${JUCE_DIR} JUCE)

# ---  console app target
juce_add_console_app(LabExercise3
    PRODUCT_NAME "LabExercise3"
)

# ---  source files
target_sources(LabExercise3 PRIVATE
    src/Main.cpp
)

# --- Link the minimal JUCE module(s) we need
target_link_libraries(LabExercise3 PRIVATE
    juce::juce_core
)

# --- Helpful warnings (clang/gcc)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(LabExercise3 PRIVATE -Wall -Wextra -Wpedantic)
endif()
