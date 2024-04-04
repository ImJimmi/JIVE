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

add_subdirectory(jive_components)
add_subdirectory(jive_core)
add_subdirectory(jive_layouts)
add_subdirectory(jive_style_sheets)

add_subdirectory(runners)
