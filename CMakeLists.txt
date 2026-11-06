cmake_minimum_required(VERSION 3.22)
project(BaiGSubBass VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(BaiGAssets SOURCES bass_hero.jpg)

set(BAIG_PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND BAIG_PLUGIN_FORMATS AU)
endif()

juce_add_plugin(BaiGSubBass
    COMPANY_NAME "Baï-G Production"
    BUNDLE_ID "com.baigproduction.subbass"
    PLUGIN_MANUFACTURER_CODE BaiG
    PLUGIN_CODE Bgsb
    FORMATS ${BAIG_PLUGIN_FORMATS}
    PRODUCT_NAME "Baï-G Sub Bass"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(BaiGSubBass PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_include_directories(BaiGSubBass PRIVATE Source)
target_link_libraries(BaiGSubBass PRIVATE
    BaiGAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
target_compile_definitions(BaiGSubBass PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)
