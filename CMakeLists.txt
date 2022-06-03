cmake_minimum_required(VERSION 3.15)
project(JIVE
    VERSION 1.0.0
)
configure_file(${CMAKE_SOURCE_DIR}/version.txt.in version.txt)

option(JIVE_BUILD_TEST_RUNNER "Build JIVE's test runner?" OFF)
option(JIVE_BUILD_DEMO_RUNNER "Build JIVE's demo runner?" OFF)

if (JIVE_BUILD_TEST_RUNNER OR JIVE_BUILD_DEMO_RUNNER)
    add_subdirectory(runners/libraries)
endif()

juce_add_module(jive_components ALIAS_NAMESPACE jive)
juce_add_module(jive_core ALIAS_NAMESPACE jive)
juce_add_module(jive_layouts ALIAS_NAMESPACE jive)
juce_add_module(jive_style_sheets ALIAS_NAMESPACE jive)

add_subdirectory(runners)
