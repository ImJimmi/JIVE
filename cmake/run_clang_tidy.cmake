if(NOT CLANG_TIDY_COMMAND)
    message(FATAL_ERROR "clang-tidy not found")
endif()

set(FILTERED_ARGS "")
set(START_OF_CLANG_TIDY_ARGS OFF)
math(EXPR last_arg "${CMAKE_ARGC} - 1")
set(LAST_ARG "")

foreach(i RANGE 0 ${last_arg})
    set(ARG "${CMAKE_ARGV${i}}")

    if(NOT START_OF_CLANG_TIDY_ARGS)
        if(ARG MATCHES ".*/run_clang_tidy.cmake$")
            set(START_OF_CLANG_TIDY_ARGS ON)
        endif()

        continue()
    endif()

    if(ARG STREQUAL "--")
        if (LAST_ARG MATCHES ".*(.cache|JuceLibraryCode|melatonin_inspector|submodules|_deps).*|.*.mm")
            cmake_language(EXIT 0)
        endif()
    endif()

    set(LAST_ARG ${ARG})

    if(ARG STREQUAL "-GL")
        continue()
    elseif(ARG MATCHES "^/GL$")
        continue()
    else()
        list(APPEND FILTERED_ARGS "${ARG}")
    endif()
endforeach()

if(JUCE_TIDY_PLUGIN)
    set(JUCE_TIDY_ARGS "--load=${JUCE_TIDY_PLUGIN}")
endif()

execute_process(
    COMMAND ${CLANG_TIDY_COMMAND} ${JUCE_TIDY_ARGS} "--header-filter=${CLANG_TIDY_HEADER_FILTER}" "--config-file=${CLANG_TIDY_CONFIG}" --extra-arg=-Wno-unknown-warning-option -p "${CLANG_TIDY_BUILD_DIR}" ${FILTERED_ARGS}
    RESULT_VARIABLE RESULT
)

if(NOT RESULT EQUAL 0)
    message(FATAL_ERROR "clang-tidy failed with exit code ${RESULT}")
endif()
