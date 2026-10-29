cmake_minimum_required(VERSION 3.22)
project(Ripple VERSION 1.0.0)

add_subdirectory(JUCE)

juce_add_plugin(Ripple
    COMPANY_NAME "Tal Mendel"
    PLUGIN_MANUFACTURER_CODE TlMd
    PLUGIN_CODE Rple
    FORMATS VST3 AU Standalone
    BUNDLE_ID com.talmendel.ripple
    PRODUCT_NAME "Ripple"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Delay Reverb Modulation)

target_sources(Ripple PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(Ripple PRIVATE cxx_std_17)
target_compile_definitions(Ripple PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)
target_link_libraries(Ripple PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
