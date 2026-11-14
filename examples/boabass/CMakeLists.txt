juce_add_plugin(jive-boabass
                PRODUCT_NAME "BoaBass"
                COMPANY_NAME "JIVE"
                FORMATS VST3 AU
                IS_SYNTH TRUE
                NEEDS_MIDI_INPUT TRUE
                PLUGIN_MANUFACTURER_CODE "Jive"
                PLUGIN_CODE "Boab"
)

juce_add_gui_app(jive-boabass-app
                 PRODUCT_NAME "BoaBass"
                 COMPANY_NAME "JIVE"
)

target_sources(jive-boabass
               PRIVATE source/PluginFilter.cpp
)

target_sources(jive-boabass-app
               PRIVATE source/App.cpp
)

target_compile_definitions(jive-boabass
                           PRIVATE JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_definitions(jive-boabass-app
                           PRIVATE JUCE_APPLICATION_NAME="$<TARGET_PROPERTY:jive-boabass-app,JUCE_PRODUCT_NAME>"
                                   JUCE_APPLICATION_VERSION="$<TARGET_PROPERTY:jive-boabass-app,JUCE_VERSION>"
)

foreach (TARGET jive-boabass jive-boabass-app)
    target_sources(${TARGET}
                   PRIVATE source/PluginProcessor.h
                           source/Presenter.h
                           source/audio/Parameters.h
                           source/audio/Presets.h
                           source/audio/Voice.h
                           source/components/EnvelopeView.h
                           source/components/Keyboard.h
                           source/components/Meter.h
                           source/graphics/ComboBoxPainter.h
                           source/graphics/KnobPainter.h
    )

    # The app hosts the processor itself, so it attaches parameters too.
    target_compile_definitions(${TARGET}
                               PRIVATE JIVE_ENABLE_MELATONIN_INSPECTOR=$<BOOL:${JIVE_ENABLE_MELATONIN_INSPECTOR}>
                                       JIVE_IS_PLUGIN_PROJECT=1
    )

    target_link_libraries(${TARGET}
                          PRIVATE examples-shared-lib
                                  jive::compiler_and_linker_options
                                  jive::jive_layouts
                                  jive::jive_style_sheets
                                  juce::juce_audio_utils
                                  juce::juce_dsp
                                  juce::juce_recommended_config_flags
                                  juce::juce_recommended_lto_flags
                                  juce::juce_recommended_warning_flags
    )

    jive_sources(${TARGET}
                 SOURCES "public/*"
    )

    if (JIVE_ENABLE_MELATONIN_INSPECTOR)
        add_melatonin_inspector(${TARGET})
        target_link_libraries(${TARGET}
                              PRIVATE melatonin_inspector
        )
    endif ()
endforeach ()

if (JIVE_ENABLE_PLUGINVAL)
    add_pluginval_tests(jive-boabass)
endif ()
