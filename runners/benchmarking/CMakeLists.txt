juce_add_console_app(jive-benchmarking
    PRODUCT_NAME "JIVE Benchmarking"
)

target_sources(jive-benchmarking
PRIVATE
    source/main.cpp
)

target_include_directories(jive-benchmarking
PRIVATE
    source
)

target_compile_definitions(jive-benchmarking
PRIVATE
    JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=0
    JIVE_UNIT_TESTS=1
    JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-benchmarking,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-benchmarking,JUCE_VERSION>"
)

target_link_libraries(jive-benchmarking
PRIVATE
    jive::compiler_and_linker_options
    jive::jive_layouts
    jive::jive_style_sheets
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
