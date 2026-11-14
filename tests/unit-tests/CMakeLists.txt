juce_add_console_app(jive-test-runner
                     PRODUCT_NAME
                     "JIVE Test Runner"
)

target_sources(jive-test-runner
               PRIVATE source/integration-tests/ButtonWithNestedIconAndText.cpp
                       source/integration-tests/CentredNestedTextInFixedWidthColumn.cpp
                       source/integration-tests/SideBarWhenWindowIsResized.cpp
                       source/main.cpp
)

# Fonts whose shaping tests depend on, such as Rubik's ligatures, so the
# tests don't depend on what's installed.
juce_add_binary_data(jive-test-runner-fonts
                     HEADER_NAME JiveTestFonts.h
                     NAMESPACE jive_test_fonts
                     SOURCES ${PROJECT_SOURCE_DIR}/examples/resources/Rubik-Regular.ttf
)

target_include_directories(jive-test-runner
                           PRIVATE source
)

target_compile_definitions(jive-test-runner
                           PRIVATE JIVE_UNIT_TESTS=1
                                   JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-test-runner,JUCE_PRODUCT_NAME>"
                                   JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-test-runner,JUCE_VERSION>"
)

target_link_libraries(jive-test-runner
                      PRIVATE jive-test-runner-fonts
                              jive::code_coverage
                              jive::compiler_and_linker_options
                              jive::jive_layouts
                              jive::jive_style_sheets
                              juce::juce_recommended_config_flags
                              juce::juce_recommended_lto_flags
                              juce::juce_recommended_warning_flags
)

add_test(NAME "jive-test-runner"
         COMMAND $<TARGET_FILE:jive-test-runner>
)
