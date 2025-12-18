include_guard()

find_program(GH_COMMAND gh REQUIRED)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/pluginval-prebuilt)
    if (APPLE)
        set(PATTERN *macOS.zip)
    else()
        set(PATTERN *Windows.zip)
    endif()

    message(STATUS "Fetching latest pluginval release...")
    execute_process(
        COMMAND ${GH_COMMAND} release download
            --repo=Tracktion/pluginval
            --pattern=${PATTERN}
            --output=${CMAKE_BINARY_DIR}/pluginval-prebuilt/archive.zip
            --skip-existing
        COMMAND_ERROR_IS_FATAL ANY
    )

    execute_process(
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/pluginval-prebuilt
        COMMAND ${CMAKE_COMMAND} -E tar xzf archive.zip
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

if (APPLE)
    set(PLUGINVAL_COMMAND ${CMAKE_BINARY_DIR}/pluginval-prebuilt/pluginval.app/Contents/MacOS/pluginval)
else()
    set(PLUGINVAL_COMMAND ${CMAKE_BINARY_DIR}/pluginval-prebuilt/pluginval.exe)
endif()

function(add_pluginval_tests TARGET)
    set(TEST_NAME "${TARGET}_VST3.pluginval")
    get_target_property(PLUGIN_ARTEFACT "${TARGET}_VST3" JUCE_PLUGIN_ARTEFACT_FILE)

    add_test(
        NAME ${TEST_NAME}
        COMMAND ${PLUGINVAL_COMMAND}
            --strictness-level 10
            --validate ${PLUGIN_ARTEFACT}
    )

    set_tests_properties (${TEST_NAME}
    PROPERTIES
        REQUIRED_FILES ${PLUGIN_ARTEFACT}
        FAIL_REGULAR_EXPRESSION ".*runtime error:.*"
    )
endfunction()
