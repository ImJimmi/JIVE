include_guard()

add_library(jive_compiler_and_linker_options
            INTERFACE
)
add_library(jive::compiler_and_linker_options
            ALIAS
            jive_compiler_and_linker_options
)

target_compile_definitions(jive_compiler_and_linker_options
                           INTERFACE JUCE_DISABLE_JUCE_VERSION_PRINTING=1
                                     JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
                                     JUCE_USE_CURL=0
                                     JUCE_WEB_BROWSER=0
)

if (APPLE)
    target_compile_options(jive_compiler_and_linker_options
                           INTERFACE -Werror
    )

    target_link_options(jive_compiler_and_linker_options
                        INTERFACE
                        -Wl,-weak_reference_mismatches,weak
    )

    if (JIVE_ENABLE_SANITISERS)
        target_compile_options(jive_compiler_and_linker_options
                               INTERFACE -fsanitize=undefined
        )

        target_link_options(jive_compiler_and_linker_options
                            INTERFACE
                            -fsanitize=undefined
        )
    endif ()
endif ()

if (MSVC)
    target_compile_options(jive_compiler_and_linker_options
                           INTERFACE /WX
    )
endif ()
