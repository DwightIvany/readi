cmake_minimum_required(VERSION 3.22)

project(di75 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Pure DSP core: no JUCE, linked by both the plugin and the golden test.
add_library(di75_dsp STATIC src/Di75Dsp.cpp src/Di75Dsp.h)
target_include_directories(di75_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

juce_add_plugin(di75
    COMPANY_NAME "Dwight Ivany"
    BUNDLE_ID com.dwightivany.di75
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx Compressor"
    FORMATS VST3
    PRODUCT_NAME "di75")

target_sources(di75 PRIVATE
    src/Di75PluginProcessor.cpp
    src/Di75PluginProcessor.h
    src/Di75PluginEditor.cpp
    src/Di75PluginEditor.h)

target_compile_definitions(di75
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(di75
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        di75_dsp)

enable_testing()
add_executable(golden_test tests/golden_test.cpp)
target_link_libraries(golden_test PRIVATE di75_dsp)

add_test(
    NAME golden
    COMMAND golden_test
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/data/cases.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/data)
