include(cmake/melatonin_inspector.cmake)
include(cmake/pluginval.cmake)

add_library(jive-demo
            INTERFACE
)

juce_add_binary_data(jive-demo-resources
                     NAMESPACE
                     "binary_data"
                     SOURCES
                     resources/icons/back-small.svg
                     resources/icons/build-large.svg
                     resources/icons/code-large.svg
                     resources/icons/layout-large.svg
                     resources/icons/motion-blur-large.svg
                     resources/icons/next-small.svg
                     resources/icons/palette-large.svg
                     resources/rubik-mono-one.ttf
                     resources/rubik.ttf
                     resources/source-code-pro.ttf
)

target_include_directories(jive-demo
                           INTERFACE source
)

target_compile_definitions(jive-demo
                           INTERFACE JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=0
                                     JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-demo,JUCE_PRODUCT_NAME>"
                                     JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-demo,JUCE_VERSION>"
                                     JUCE_VST3_CAN_REPLACE_VST2=0
                                     JIVE_ENABLE_MELATONIN_INSPECTOR=$<BOOL:${JIVE_ENABLE_MELATONIN_INSPECTOR}>
)

target_link_libraries(jive-demo
                      INTERFACE jive-demo-resources
                                jive::compiler_and_linker_options
                                jive::jive_layouts
                                juce::juce_recommended_config_flags
                                juce::juce_recommended_lto_flags
                                juce::juce_recommended_warning_flags
)

if (JIVE_ENABLE_MELATONIN_INSPECTOR)
    add_melatonin_inspector(jive-demo)
endif ()

juce_add_gui_app(jive-demo-app
                 PRODUCT_NAME
                 "JIVE Demo"
)

target_sources(jive-demo-app
               PRIVATE source/main.cpp
)

target_link_libraries(jive-demo-app
                      PRIVATE jive-demo
)

juce_add_plugin(jive-demo-plugin
                PRODUCT_NAME
                "JIVE Demo"
                FORMATS
                Standalone
                VST3
                AU
)

target_sources(jive-demo-plugin
               PRIVATE source/PluginFilter.cpp
)

target_compile_definitions(jive-demo-plugin
                           PRIVATE JIVE_IS_PLUGIN_PROJECT=1
)

target_link_libraries(jive-demo-plugin
                      PRIVATE jive-demo
                              juce::juce_audio_utils
                              juce::juce_audio_devices
)

add_pluginval_tests(jive-demo-plugin)
