juce_add_plugin(jive-gain-example
                PRODUCT_NAME
                "JIVE Gain Example"
                VERSION
                ${PROJECT_VERSION}
                FORMATS
                Standalone
                VST3
                AU
                PLUGIN_MANUFACTURER_CODE
                Jive
                PLUGIN_CODE
                Gain
)

target_sources(jive-gain-example
               PUBLIC source/CreatePluginFilter.cpp
                      source/Processor.h
                      source/View.h
)

target_compile_definitions(jive-gain-example
                           PRIVATE JIVE_IS_PLUGIN_PROJECT=1
                                   JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS=1
                                   JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(jive-gain-example
                      PRIVATE jive::compiler_and_linker_options
                              jive::jive_layouts
                              jive::jive_style_sheets
                              juce::juce_audio_devices
                              juce::juce_audio_utils
                              juce::juce_recommended_config_flags
                              juce::juce_recommended_lto_flags
                              juce::juce_recommended_warning_flags
)
