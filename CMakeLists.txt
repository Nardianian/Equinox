# Sets the minimum required version of CMake for the project
cmake_minimum_required(VERSION 3.24)

# Sets the project name and version
project(Equinox VERSION 1.2.0)

# Forza l'uso del runtime statico (MT per Release, MTd per Debug) su MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(EQUINOX_BUILD_TESTS "Build Equinox core regression tests" ON)

# Adds the JUCE library
add_subdirectory(lib/JUCE)

juce_set_vst2_sdk_path("C:/SDKs/vstsdk2.4")
juce_set_aax_sdk_path("C:/SDKs/aax-sdk")

include_directories(C:/SDKs/asiosdk/common)
include_directories("C:/Program Files/JACK2/include")

# `juce_add_plugin` adds a static library target with the name passed as the first argument
# (AudioPluginExample here). This target is a normal CMake target, but has a lot of extra properties set
# up by default. As well as this shared code static library, this function adds targets for each of
# the formats specified by the FORMATS arguments. This function accepts many optional arguments.
# Check the readme at `docs/CMake API.md` in the JUCE repo for the full list.
juce_add_plugin("${PROJECT_NAME}"
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME "OSR"                          # Specify the name of the plugin's author
    IS_SYNTH TRUE                               # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE                       # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT FALSE                     # Does the plugin need midi output?
    IS_MIDI_EFFECT FALSE                        # Is this plugin a MIDI effect?
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE           # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD FALSE               # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE Oras               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Eqnx                            # A unique four-character plugin id with exactly one upper-case character
    FORMATS AAX VST3 VST Standalone LV2         # The formats to build. Valid formats are: Standalone AAX Unity VST VST3 AU AUv3
    DOCUMENT_EXTENSIONS eqnx equinox            # The document extension(s) to associate with this plugin
    PRODUCT_NAME "${PROJECT_NAME}"              # The name of the final executable, which can differ from the target name
    VST3_CATEGORIES "Instrument" "Synth"
    VST2_CATEGORY kPlugCategSynth
    AAX_EPlugInCategory_SWGenerators
    BUNDLE_ID "com.OliverRasmussen.Equinox"
    LV2URI https://github.com/OliverRasmussen/Equinox
    AAX_IDENTIFIER com.OliverRasmussen.Equinox
)             


# `juce_generate_juce_header` will create a JuceHeader.h for a given target, which will be generated
# into your build tree. This should be included with `#include <JuceHeader.h>`. The include path for
# this header will be automatically added to the target. The main function of the JuceHeader is to
# include all your JUCE module headers; if you're happy to include module headers directly, you
# probably don't need to call this.
juce_generate_juce_header("${PROJECT_NAME}")

# `target_sources` adds source files to a target. We pass the target that needs the sources as the
# first argument, then a visibility parameter for the sources which should normally be PRIVATE.
# Finally, we supply a list of source files that will be built into the target. This is a standard
# CMake command.

# Creates a list of all the source files in the src folder recursively
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
# Adds the source files to the target
target_sources("${PROJECT_NAME}" PRIVATE ${SOURCE_FILES})

target_include_directories("${PROJECT_NAME}" PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Sets the C++ standard of the target to C++20
target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)

# `target_compile_definitions` adds some preprocessor definitions to our target. In a Projucer
# project, these might be passed in the 'Preprocessor Definitions' field. JUCE modules also make use
# of compile definitions to switch certain features on/off, so if there's a particular feature you
# need that's not on by default, check the module header for the correct flag to set here. These
# definitions will be visible both to your code, and also the JUCE module code, so for new
# definitions, pick unique names that are unlikely to collide! This is a standard CMake command.
target_compile_definitions("${PROJECT_NAME}"
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0                 # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0                    # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_ASIO=1
    JUCE_JACK=1
    JUCE_USE_WINRT_MIDI=1)

# If your target needs extra binary assets, you can add them here. The first argument is the name of
# a new static library target that will include all the binary resources. There is an optional
# `NAMESPACE` argument that can specify the namespace of the generated binary data class. Finally,
# the SOURCES argument should be followed by a list of source files that should be built into the
# static library. These source files can be of any kind (wav data, images, fonts, icons etc.).
# Conversion to binary-data will happen when your target is built.

file(GLOB_RECURSE GRAPHICS_RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/graphics/*")
list(FILTER GRAPHICS_RESOURCE_FILES EXCLUDE REGEX ".*DS_Store")

file(GLOB_RECURSE FONT_RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/fonts/*")
list(FILTER FONT_RESOURCE_FILES EXCLUDE REGEX ".*DS_Store")

file(GLOB_RECURSE PRESET_RESOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/presets/*.equinox")

juce_add_binary_data(GraphicsResources
    HEADER_NAME GraphicsBinaryData.h
    SOURCES ${GRAPHICS_RESOURCE_FILES}
    NAMESPACE GraphicsResources)

juce_add_binary_data(FontResources
    HEADER_NAME FontBinaryData.h
    SOURCES ${FONT_RESOURCE_FILES}
    NAMESPACE FontResources)

juce_add_binary_data(PresetResources
    HEADER_NAME PresetBinaryData.h
    SOURCES ${PRESET_RESOURCE_FILES}
    NAMESPACE PresetResources)

# `target_link_libraries` links libraries and JUCE modules to other libraries or executables. Here,
# we're linking our executable target to the `juce::juce_audio_utils` module. Inter-module
# dependencies are resolved automatically, so `juce_core`, `juce_events` and so on will also be
# linked automatically. If we'd generated a binary data target above, we would need to link to it
# here too. This is a standard CMake command.
target_link_libraries("${PROJECT_NAME}"
    PRIVATE
    GraphicsResources
    FontResources
    PresetResources
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_devices
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

if (EQUINOX_BUILD_TESTS)
    enable_testing()

    juce_add_console_app(EquinoxCoreTests)
    juce_generate_juce_header(EquinoxCoreTests)

    target_sources(EquinoxCoreTests PRIVATE
        tests/EquinoxCoreTests.cpp)

    target_include_directories(EquinoxCoreTests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_GraphicsResources/JuceLibraryCode"
        "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_FontResources/JuceLibraryCode"
        "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_PresetResources/JuceLibraryCode")

    target_link_libraries(EquinoxCoreTests
        PRIVATE
        Equinox
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(EquinoxCoreTests PRIVATE -Wno-macro-redefined)
    endif()

    add_test(NAME EquinoxCoreTests COMMAND EquinoxCoreTests)
endif()
