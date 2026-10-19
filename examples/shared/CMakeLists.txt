juce_add_binary_data(examples-shared-resources
NAMESPACE
    "shared_resources"
HEADER_NAME
    "SharedResources.h"
SOURCES
    ../resources/rubik.ttf
    ../resources/rubik-mono-one.ttf
    ../resources/source-code-pro.ttf
)

add_library(examples-shared-lib INTERFACE)

target_sources(examples-shared-lib
INTERFACE
    shared/Application.h
    shared/Fonts.h
    shared/Inspector.h
    shared/Presenter.h
    shared/Theme.h
)

target_include_directories(examples-shared-lib
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(examples-shared-lib
INTERFACE
    examples-shared-resources
    jive::compiler_and_linker_options
    jive::jive_layouts
    jive::jive_style_sheets
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
