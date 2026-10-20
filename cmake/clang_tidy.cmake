include_guard()

find_program(CLANG_TIDY_COMMAND clang-tidy)

if (NOT CLANG_TIDY_COMMAND)
    if (APPLE)
        message(FATAL_ERROR "clang-tidy not found. Install using\n  $ brew install llvm\n  Then link it with\n  $ sudo ln -sf $(brew --prefix llvm)/bin/clang-tidy /usr/local/bin/clang-tidy")
    elseif(WIN32)
        message(FATAL_ERROR "clang-tidy not found. Install using\n  $ winget install LLVM.LLVM\n  Or install the 'C++ Clang tools for Windows' Visual Studio component")
    endif()
endif()

if (APPLE)
    execute_process(
        COMMAND "${CLANG_TIDY_COMMAND}" --version
        OUTPUT_VARIABLE CLANG_TIDY_VERSION_OUTPUT
    )
    string(REGEX MATCH "version ([0-9]+)" CLANG_TIDY_VERSION "${CLANG_TIDY_VERSION_OUTPUT}")
    set(CLANG_TIDY_VERSION_MAJOR "${CMAKE_MATCH_1}")

    set(JUCE_TIDY_ASSET "JuceTidy-clang-tidy-${CLANG_TIDY_VERSION_MAJOR}-macos-${CMAKE_HOST_SYSTEM_PROCESSOR}")
    set(JUCE_TIDY_DIR "${PROJECT_BINARY_DIR}/${JUCE_TIDY_ASSET}")
    set(JUCE_TIDY_PLUGIN "${JUCE_TIDY_DIR}/JuceTidy.so")

    if (NOT EXISTS "${JUCE_TIDY_PLUGIN}")
        file(DOWNLOAD
            "https://github.com/ImJimmi/JuceTidy/releases/latest/download/${JUCE_TIDY_ASSET}.zip"
            "${JUCE_TIDY_DIR}.zip"
            STATUS JUCE_TIDY_DOWNLOAD_STATUS
        )
        list(GET JUCE_TIDY_DOWNLOAD_STATUS 0 JUCE_TIDY_DOWNLOAD_RESULT)

        if (JUCE_TIDY_DOWNLOAD_RESULT EQUAL 0)
            file(ARCHIVE_EXTRACT INPUT "${JUCE_TIDY_DIR}.zip" DESTINATION "${JUCE_TIDY_DIR}")
        else()
            message(WARNING "No JuceTidy release found for clang-tidy ${CLANG_TIDY_VERSION_MAJOR} on ${CMAKE_HOST_SYSTEM_PROCESSOR}. Running clang-tidy without JuceTidy.")
            set(JUCE_TIDY_PLUGIN "")
        endif()

        file(REMOVE "${JUCE_TIDY_DIR}.zip")
    endif()
endif()

function(add_clang_tidy TARGET)
    set(OPTIONS "")
    set(ONE_VALUE_KEYWORDS HEADER_FILTER)
    set(MULTI_VALUE_KEYWORDS "")
    cmake_parse_arguments (CLANG_TIDY "${OPTIONS}" "${ONE_VALUE_KEYWORDS}" "${MULTI_VALUE_KEYWORDS}" ${ARGN})

    if (NOT CLANG_TIDY_HEADER_FILTER)
        set(CLANG_TIDY_HEADER_FILTER "${PROJECT_SOURCE_DIR}/(jive_core|jive_layouts|jive_style_sheets|examples|tests)/")
    endif()

    set(CLANG_TIDY_PROPERTIES
        "${CMAKE_COMMAND}"
        "-DCLANG_TIDY_COMMAND=${CLANG_TIDY_COMMAND}"
        "-DCLANG_TIDY_HEADER_FILTER=${CLANG_TIDY_HEADER_FILTER}"
        "-DCLANG_TIDY_CONFIG=${PROJECT_SOURCE_DIR}/.clang-tidy"
        "-DCLANG_TIDY_BUILD_DIR=${PROJECT_BINARY_DIR}"
        "-DJUCE_TIDY_PLUGIN=${JUCE_TIDY_PLUGIN}"
        -P
        "${PROJECT_SOURCE_DIR}/cmake/run_clang_tidy.cmake"
    )

    set_target_properties(${TARGET}
    PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_PROPERTIES}"
    )
endfunction()
