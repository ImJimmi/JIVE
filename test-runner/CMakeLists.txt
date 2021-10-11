juce_add_console_app(jive-test-runner
    VERSION 0.1.0
)

target_sources(jive-test-runner
PRIVATE
    source/main.cpp
)

target_link_libraries(jive-test-runner
PRIVATE
    juce::juce_core
    jive::jive
)

target_include_directories(jive-test-runner
PRIVATE
    source
)

target_compile_features(jive-test-runner
PRIVATE
    cxx_std_14
)

target_compile_options(jive-test-runner
PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

get_target_property(JIVE_TEST_RUNNER_SOURCES jive-test-runner SOURCES)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${JIVE_TEST_RUNNER_SOURCES}
)
