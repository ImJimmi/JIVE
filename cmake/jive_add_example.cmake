include_guard()

function(jive_add_example TARGET)
    set(OPTIONS APP PLUGIN)
    set(ONE_VALUE_ARGS PRODUCT_NAME PLUGIN_CODE)
    set(MULTI_VALUE_ARGS SOURCES RESOURCES)
    cmake_parse_arguments(EXAMPLE "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if ("${EXAMPLE_APP}" STREQUAL "${EXAMPLE_PLUGIN}")
        message(FATAL_ERROR "jive_add_example(${TARGET}) needs exactly one of APP or PLUGIN")
    endif ()

    if (EXAMPLE_APP)
        juce_add_gui_app(${TARGET}
                         PRODUCT_NAME "${EXAMPLE_PRODUCT_NAME}"
        )

        target_compile_definitions(${TARGET}
                                   PRIVATE JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:${TARGET},JUCE_PRODUCT_NAME>"
                                           JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:${TARGET},JUCE_VERSION>"
        )
    else ()
        if (NOT EXAMPLE_PLUGIN_CODE)
            message(FATAL_ERROR "jive_add_example(${TARGET}) needs a PLUGIN_CODE for a PLUGIN")
        endif ()

        juce_add_plugin(${TARGET}
                        PRODUCT_NAME "${EXAMPLE_PRODUCT_NAME}"
                        FORMATS Standalone VST3 AU
                        PLUGIN_MANUFACTURER_CODE "Jive"
                        PLUGIN_CODE "${EXAMPLE_PLUGIN_CODE}"
        )

        target_compile_definitions(${TARGET}
                                   PRIVATE JIVE_IS_PLUGIN_PROJECT=1
                                           JUCE_VST3_CAN_REPLACE_VST2=0
        )

        target_link_libraries(${TARGET}
                              PRIVATE juce::juce_audio_devices
                                      juce::juce_audio_utils
        )
    endif ()

    target_sources(${TARGET}
                   PRIVATE ${EXAMPLE_SOURCES}
    )

    target_compile_definitions(${TARGET}
                               PRIVATE JIVE_ENABLE_MELATONIN_INSPECTOR=$<BOOL:${JIVE_ENABLE_MELATONIN_INSPECTOR}>
    )

    target_link_libraries(${TARGET}
                          PRIVATE jive::compiler_and_linker_options
                                  jive::jive_layouts
                                  jive::jive_style_sheets
                                  juce::juce_recommended_config_flags
                                  juce::juce_recommended_lto_flags
                                  juce::juce_recommended_warning_flags
    )

    if (EXAMPLE_RESOURCES)
        juce_add_binary_data(${TARGET}-resources
                             NAMESPACE
                             "binary_data"
                             SOURCES
                             ${EXAMPLE_RESOURCES}
        )

        target_link_libraries(${TARGET}
                              PRIVATE ${TARGET}-resources
        )
    endif ()

    if (JIVE_ENABLE_MELATONIN_INSPECTOR)
        add_melatonin_inspector(${TARGET})
        target_link_libraries(${TARGET}
                              PRIVATE melatonin_inspector
        )
    endif ()

    if (EXAMPLE_PLUGIN AND JIVE_ENABLE_PLUGINVAL)
        add_pluginval_tests(${TARGET})
    endif ()
endfunction()
