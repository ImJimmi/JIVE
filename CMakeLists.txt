cmake_minimum_required(VERSION 3.15)

option(JIVE_BUILD_TEST_RUNNER "Whether or not to build JIVE's test runner" ON)

if (JIVE_BUILD_TEST_RUNNER)
    project(JIVE)

    set(DEFAULT_XCODE_GENERATE_SCHEME ${CMAKE_XCODE_GENERATE_SCHEME})

    set(CMAKE_XCODE_GENERATE_SCHEME OFF)
    set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)

    add_subdirectory(test-runner/libraries/JUCE)

    set(CMAKE_XCODE_GENERATE_SCHEME ${DEFAULT_XCODE_GENERATE_SCHEME})

endif()

juce_add_module(jive ALIAS_NAMESPACE jive)

if (JIVE_BUILD_TEST_RUNNER)
    add_subdirectory(test-runner)
endif()
