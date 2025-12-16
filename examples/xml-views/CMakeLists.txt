juce_add_gui_app(jive-xml-views
                 PRODUCT_NAME
                 "XML Views"
)

target_sources(jive-xml-views
               PRIVATE main.cpp
)

target_link_libraries(jive-xml-views
                      PRIVATE jive::compiler_and_linker_options
                              jive::jive_layouts
                              jive::jive_style_sheets
                              juce::juce_recommended_config_flags
                              juce::juce_recommended_lto_flags
                              juce::juce_recommended_warning_flags
)

target_compile_definitions(jive-xml-views
                           PRIVATE JIVE_XML_VIEWS_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
                                   JIVE_ENABLE_MELATONIN_INSPECTOR=$<BOOL:${JIVE_ENABLE_MELATONIN_INSPECTOR}>)

if (JIVE_ENABLE_MELATONIN_INSPECTOR)
    add_melatonin_inspector(jive-xml-views)
    target_link_libraries(jive-xml-views PRIVATE melatonin_inspector)
endif ()
