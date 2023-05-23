juce_add_console_app(jive-test-runner
    VERSION 0.1.0
)

target_sources(jive-test-runner
PRIVATE
    source/main.cpp
)

target_link_libraries(jive-test-runner
PRIVATE
    jive::jive_layouts
    jive::jive_style_sheets
)

target_include_directories(jive-test-runner
PRIVATE
    source
)

target_compile_features(jive-test-runner
PRIVATE
    cxx_std_17
)

if (APPLE)
    target_compile_options(jive-test-runner
    PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -Wall
        -Werror
        -Wextra
        -Wpedantic
    )

    target_link_options(jive-test-runner
    PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
elseif(MSVC)
    target_compile_options(jive-test-runner
    PRIVATE
        /W4
        /WX
    )
endif()

target_compile_definitions(jive-test-runner
PRIVATE
    JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=0
    JIVE_UNIT_TESTS=1
    JUCE_APPLICATION_NAME="JIVE Test Runner"
    JUCE_DISABLE_JUCE_VERSION_PRINTING=1
)

if (APPLE)
    option(JIVE_ENABLE_COVERAGE OFF "Whether or not to enable coverage reports when building the JIVE test-runner.")
    if (JIVE_ENABLE_COVERAGE)
        target_compile_options(jive-test-runner
        PRIVATE
            --coverage
        )

        target_link_options(jive-test-runner
        PRIVATE
            --coverage
        )
    endif()
endif()

get_target_property(JIVE_TEST_RUNNER_SOURCES jive-test-runner SOURCES)
