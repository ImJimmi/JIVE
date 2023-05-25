cmake_minimum_required(VERSION 3.15)

option(JIVE_BUILD_TEST_RUNNER "Whether or not to build JIVE's test runner" OFF)

if (JIVE_BUILD_TEST_RUNNER)
    project(JIVE)
    add_subdirectory(runners/libraries)
endif()

juce_add_module(jive_components ALIAS_NAMESPACE jive)
juce_add_module(jive_core ALIAS_NAMESPACE jive)
juce_add_module(jive_layouts ALIAS_NAMESPACE jive)
juce_add_module(jive_style_sheets ALIAS_NAMESPACE jive)

add_subdirectory(runners)
