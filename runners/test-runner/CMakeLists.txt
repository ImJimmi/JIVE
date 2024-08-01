juce_add_console_app(jive-test-runner
                     PRODUCT_NAME
                     "JIVE Test Runner"
)

target_sources(jive-test-runner
               PRIVATE source/integration-tests/ButtonWithNestedIconAndText.cpp
                       source/main.cpp
)

target_include_directories(jive-test-runner
                           PRIVATE source
)

target_compile_definitions(jive-test-runner
                           PRIVATE JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=0
                                   JIVE_UNIT_TESTS=1
                                   JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-test-runner,JUCE_PRODUCT_NAME>"
                                   JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-test-runner,JUCE_VERSION>"
)

target_link_libraries(jive-test-runner
                      PRIVATE jive::code_coverage
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
