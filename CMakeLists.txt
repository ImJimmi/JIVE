cmake_minimum_required(VERSION 3.15)
project(JIVE
        VERSION 2.0.0
)
configure_file(version.txt.in
               version.txt
)

include(cmake/jive_options.cmake)
include(cmake/jive_code_coverage.cmake)
include(cmake/jive_compiler_and_linker_options.cmake)

if (JIVE_BUILD_BENCHMARKS
    OR JIVE_BUILD_DEMO_RUNNER
    OR JIVE_BUILD_TEST_RUNNER
)
    include(CTest)
    include(cmake/CPM.cmake)
    CPMUsePackageLock(package-lock.cmake)
endif ()

add_subdirectory(jive_components)
add_subdirectory(jive_core)
add_subdirectory(jive_layouts)
add_subdirectory(jive_style_sheets)

if (JIVE_BUILD_BENCHMARKS
    OR JIVE_BUILD_DEMO_RUNNER
    OR JIVE_BUILD_TEST_RUNNER
    OR JIVE_BUILD_EXAMPLES
)
    CPMGetPackage(JUCE)
endif ()

set_directory_properties(PROPERTIES JUCE_COMPANY_NAME
                                    "JIVE"
                                    JUCE_COMPANY_WEBSITE
                                    "https://github.com/ImJimmi/JIVE"
)

add_subdirectory(runners)

if (JIVE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
