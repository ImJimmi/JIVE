juce_add_console_app(jive-test-runner
    VERSION 0.1.0
)

add_test(
    NAME jive-tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/jive-test-runner_artefacts/jive-test-runner
)

target_sources(jive-test-runner
PRIVATE
    source/main.cpp
)

target_link_libraries(jive-test-runner
PRIVATE
    jive::jive_layouts
    juce::juce_gui_basics
)

target_include_directories(jive-test-runner
PRIVATE
    source
)

target_compile_features(jive-test-runner
PRIVATE
    cxx_std_14
)

if (APPLE)
    target_compile_options(jive-test-runner
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>

        -fsanitize=address
        -fsanitize=undefined
    )
endif()

target_link_options(jive-test-runner
PRIVATE
    -fsanitize=address
    -fsanitize=undefined
)

target_compile_definitions(jive-test-runner
PRIVATE
    JUCE_DISABLE_JUCE_VERSION_PRINTING=1
    JIVE_UNIT_TESTS=1
    JUCE_APPLICATION_NAME="JIVE Test Runner"
)

get_target_property(JIVE_TEST_RUNNER_SOURCES jive-test-runner SOURCES)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${JIVE_TEST_RUNNER_SOURCES}
)
