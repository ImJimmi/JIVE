cmake_minimum_required(VERSION 3.15)
project(JIVE
        VERSION 2.0.0
)
configure_file(version.txt.in
               version.txt
)
set_directory_properties(
    PROPERTIES
        JUCE_COMPANY_NAME "JIVE"
        JUCE_COMPANY_WEBSITE "https://github.com/ImJimmi/JIVE"
)

option(JIVE_BUILD_BENCHMARKS "Build JIVE's benchmarks?" OFF)
option(JIVE_BUILD_EXAMPLES "Build JIVE's examples?" OFF)
option(JIVE_BUILD_TEST_RUNNER "Build JIVE's test runner?" OFF)
option(JIVE_ENABLE_COVERAGE "Generate coverage reports when running tests?" OFF)
option(JIVE_ENABLE_MELATONIN_INSPECTOR "Add the component inspector to the demos and examples?" OFF)
option(JIVE_ENABLE_PLUGINVAL "Add pluginval tests for the demo plugins?" OFF)
option(JIVE_ENABLE_PROJUCER_PROJECTS "Add Projucer targets?" OFF)

include(cmake/jive_code_coverage.cmake)
include(cmake/jive_compiler_and_linker_options.cmake)

if (JIVE_BUILD_BENCHMARKS
    OR JIVE_BUILD_TEST_RUNNER
    OR JIVE_BUILD_EXAMPLES
)
    include(cmake/CPM.cmake)
    CPMUsePackageLock(package-lock.cmake)
    CPMGetPackage(JUCE)
endif ()

if (JIVE_ENABLE_MELATONIN_INSPECTOR)
    include(cmake/melatonin_inspector.cmake)
endif()

if (JIVE_ENABLE_PERFETTO)
    include(cmake/melatonin_perfetto.cmake)
endif()

if (JIVE_ENABLE_PLUGINVAL)
    include(cmake/pluginval.cmake)
endif()

if (JIVE_ENABLE_PROJUCER_PROJECTS)
    include(cmake/projucer.cmake)
endif()

if (JIVE_BUILD_BENCHMARKS
    OR JIVE_BUILD_DEMO_RUNNER
    OR JIVE_BUILD_TEST_RUNNER
)
    include(CTest)
endif ()

add_subdirectory(jive_core)
add_subdirectory(jive_layouts)
add_subdirectory(jive_style_sheets)

if (JIVE_BUILD_TEST_RUNNER)
    add_subdirectory(tests/unit-tests)
endif()

if (JIVE_BUILD_BENCHMARKS)
    add_subdirectory(tests/benchmarks)
endif()

if (JIVE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
