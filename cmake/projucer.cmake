include_guard()

find_program(GH_COMMAND gh REQUIRED)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/JUCE-prebuilt/JUCE)
    if (APPLE)
        set(PATTERN *osx.zip)
    else()
        set(PATTERN *windows.zip)
    endif()

    message(STATUS "Fetching latest Projucer release...")
    execute_process(
        COMMAND ${GH_COMMAND} release download
            --repo=juce-framework/JUCE
            --pattern=${PATTERN}
            --output=${CMAKE_BINARY_DIR}/JUCE-prebuilt/archive.zip
            --skip-existing
        COMMAND_ERROR_IS_FATAL ANY
    )

    execute_process(
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/JUCE-prebuilt
        COMMAND ${CMAKE_COMMAND} -E tar xzf archive.zip
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

if (APPLE)
    set(PROJUCER_COMMAND ${CMAKE_BINARY_DIR}/JUCE-prebuilt/JUCE/Projucer.app/Contents/MacOS/Projucer)
    set(PROJUCER_OS osx)
else()
    set(PROJUCER_COMMAND ${CMAKE_BINARY_DIR}/JUCE-prebuilt/JUCE/Projucer.exe)
    set(PROJUCER_OS windows)
endif()

execute_process(
    COMMAND ${PROJUCER_COMMAND} --set-global-search-path ${PROJUCER_OS} defaultJuceModulePath ${CMAKE_BINARY_DIR}/JUCE-prebuilt/JUCE/modules
    COMMAND_ERROR_IS_FATAL ANY
)

function(add_projucer_target JUCER_FILE)
    if (NOT APPLE)
        message(WARNING "Projucer builds only currently implemented on macOS")
        return()
    endif()

    find_program(XCODEBUILD_COMMAND xcodebuild REQUIRED)

    get_filename_component(WORKING_DIRECTORY ${JUCER_FILE} DIRECTORY)
    get_filename_component(FILENAME ${JUCER_FILE} NAME_WLE)

    execute_process(
        COMMAND ${PROJUCER_COMMAND} --resave ${JUCER_FILE}
        COMMAND_ERROR_IS_FATAL ANY
    )

    add_custom_target(${FILENAME}_jucer ALL
        COMMAND ${XCODEBUILD_COMMAND}
            -project ${FILENAME}.xcodeproj
            -scheme "${FILENAME} - App"
            -configuration Debug
            -derivedDataPath xcode-derived
        WORKING_DIRECTORY ${WORKING_DIRECTORY}/Builds/MacOSX
        COMMENT "Building Xcode project"
    )
endfunction()
