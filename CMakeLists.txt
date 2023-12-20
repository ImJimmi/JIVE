cmake_minimum_required(VERSION 3.15)
project(JIVE
    VERSION 1.0.0
)
configure_file(version.txt.in version.txt)

include(cmake/jive_options.cmake)
include(cmake/jive_code_coverage.cmake)
include(cmake/jive_compiler_and_linker_options.cmake)

if (JIVE_BUILD_TEST_RUNNER OR JIVE_BUILD_DEMO_RUNNER)
    add_subdirectory(runners/libraries)
endif()

juce_add_module(jive_components ALIAS_NAMESPACE jive)
juce_add_module(jive_core ALIAS_NAMESPACE jive)
juce_add_module(jive_layouts ALIAS_NAMESPACE jive)
juce_add_module(jive_style_sheets ALIAS_NAMESPACE jive)

add_subdirectory(runners)
