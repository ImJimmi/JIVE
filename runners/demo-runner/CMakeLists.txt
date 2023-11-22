juce_add_gui_app(jive-demo-runner
    PRODUCT_NAME "JIVE Demo Runner"
)

target_sources(jive-demo-runner
PRIVATE
    source/main.cpp
)

juce_add_binary_data(jive-demo-runner-resources
    HEADER_NAME "binary_data.h"
    NAMESPACE "binary_data"

SOURCES
    resources/icons/back-small.svg
    resources/icons/build-large.svg
    resources/icons/build-small.svg
    resources/icons/code-large.svg
    resources/icons/code-small.svg
    resources/icons/layout-large.svg
    resources/icons/layout-small.svg
    resources/icons/next-small.svg
    resources/icons/palette-large.svg
    resources/icons/palette-small.svg
    resources/rubik-mono-one.ttf
    resources/rubik.ttf
    resources/source-code-pro.ttf
)

target_include_directories(jive-demo-runner
PRIVATE
    source
)

target_compile_definitions(jive-demo-runner
PRIVATE
    JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=1
    JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-demo-runner,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-demo-runner,JUCE_VERSION>"
)

target_link_libraries(jive-demo-runner
PRIVATE
    jive-demo-runner-resources
    jive::compiler_and_linker_options
    jive::jive_layouts
    jive::jive_style_sheets
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
