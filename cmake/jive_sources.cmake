include_guard()

# jive_sources(<target>
#              SOURCES <glob>...
#              [BUNDLE_SOURCES <bool>])
#
# With BUNDLE_SOURCES, the files matching the globs are bundled into a zip
# that's embedded in the target, and are read from there instead of from disk.
# Without it, they're read from disk so they can be hot-reloaded. Either way,
# the target refers to the files by their paths on disk. It defaults to OFF for
# Debug builds, and ON for other configurations.
function(jive_sources TARGET)
    cmake_parse_arguments(ARG "" "BUNDLE_SOURCES" "SOURCES" ${ARGN})

    if (ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "jive_sources(${TARGET}) was given unexpected arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif ()

    if (NOT ARG_SOURCES)
        message(FATAL_ERROR "jive_sources(${TARGET}) needs at least one glob in SOURCES")
    endif ()

    if ("BUNDLE_SOURCES" IN_LIST ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "jive_sources(${TARGET}) needs a value for BUNDLE_SOURCES")
    endif ()

    if (NOT DEFINED ARG_BUNDLE_SOURCES)
        set(ARG_BUNDLE_SOURCES "$<NOT:$<CONFIG:Debug>>")
    endif ()

    file(GLOB_RECURSE JIVE_SOURCES
         LIST_DIRECTORIES false
         RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
         CONFIGURE_DEPENDS
         ${ARG_SOURCES}
    )

    if (NOT JIVE_SOURCES)
        message(FATAL_ERROR "jive_sources(${TARGET}) didn't find any files matching ${ARG_SOURCES}")
    endif ()

    foreach (JIVE_SOURCE IN LISTS JIVE_SOURCES)
        if (JIVE_SOURCE MATCHES "^\\.\\./")
            message(FATAL_ERROR "jive_sources(${TARGET}) can only bundle files in ${CMAKE_CURRENT_SOURCE_DIR}, which ${JIVE_SOURCE} isn't")
        endif ()
    endforeach ()

    get_target_property(JIVE_SOURCES_COUNT ${TARGET} JIVE_SOURCES_COUNT)

    if (NOT JIVE_SOURCES_COUNT)
        set(JIVE_SOURCES_COUNT 0)
    endif ()

    math(EXPR JIVE_SOURCES_COUNT "${JIVE_SOURCES_COUNT} + 1")
    set_target_properties(${TARGET} PROPERTIES JIVE_SOURCES_COUNT ${JIVE_SOURCES_COUNT})

    string(MAKE_C_IDENTIFIER "${TARGET}_jive_sources_${JIVE_SOURCES_COUNT}" JIVE_SOURCES_NAME)
    string(TOUPPER "${JIVE_SOURCES_NAME}_BUNDLED" JIVE_SOURCES_BUNDLED_MACRO)
    set(JIVE_SOURCES_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${JIVE_SOURCES_NAME}")
    set(JIVE_SOURCES_ZIP "${JIVE_SOURCES_DIRECTORY}/sources.zip")
    file(MAKE_DIRECTORY "${JIVE_SOURCES_DIRECTORY}")

    list(TRANSFORM JIVE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/" OUTPUT_VARIABLE JIVE_SOURCES_ABSOLUTE)

    add_custom_command(OUTPUT "${JIVE_SOURCES_ZIP}"
                       COMMAND "${CMAKE_COMMAND}" -E tar cf "${JIVE_SOURCES_ZIP}" --format=zip -- ${JIVE_SOURCES}
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                       DEPENDS ${JIVE_SOURCES_ABSOLUTE}
                       COMMENT "Bundling JIVE sources for ${TARGET}"
                       VERBATIM
    )

    juce_add_binary_data(${JIVE_SOURCES_NAME}
                         NAMESPACE "${JIVE_SOURCES_NAME}"
                         HEADER_NAME "${JIVE_SOURCES_NAME}.h"
                         SOURCES "${JIVE_SOURCES_ZIP}"
    )

    set(JIVE_SOURCES_REGISTRATION "${JIVE_SOURCES_DIRECTORY}/${JIVE_SOURCES_NAME}.cpp")
    configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/jive_sources.cpp.in"
                   "${JIVE_SOURCES_REGISTRATION}"
                   @ONLY
    )

    # The registration is compiled into the target itself because the linker
    # would drop it from a static library, as nothing refers to it. A JUCE
    # plugin's target is the static library its formats share, so the
    # registration goes into each format instead.
    get_target_property(JIVE_SOURCES_IS_PLUGIN ${TARGET} JUCE_IS_PLUGIN)

    if (JIVE_SOURCES_IS_PLUGIN)
        set(JIVE_SOURCES_TARGETS)
        get_target_property(JIVE_SOURCES_FORMATS ${TARGET} JUCE_FORMATS)

        foreach (JIVE_SOURCES_FORMAT IN LISTS JIVE_SOURCES_FORMATS)
            if (TARGET ${TARGET}_${JIVE_SOURCES_FORMAT})
                list(APPEND JIVE_SOURCES_TARGETS ${TARGET}_${JIVE_SOURCES_FORMAT})
            endif ()
        endforeach ()
    else ()
        set(JIVE_SOURCES_TARGETS ${TARGET})
    endif ()

    foreach (JIVE_SOURCES_TARGET IN LISTS JIVE_SOURCES_TARGETS)
        target_sources(${JIVE_SOURCES_TARGET}
                       PRIVATE "${JIVE_SOURCES_REGISTRATION}"
        )

        target_include_directories(${JIVE_SOURCES_TARGET}
                                   PRIVATE $<TARGET_PROPERTY:jive::jive_core,INTERFACE_INCLUDE_DIRECTORIES>
        )

        target_compile_definitions(${JIVE_SOURCES_TARGET}
                                   PRIVATE ${JIVE_SOURCES_BUNDLED_MACRO}=$<BOOL:${ARG_BUNDLE_SOURCES}>
        )

        target_link_libraries(${JIVE_SOURCES_TARGET}
                              PRIVATE ${JIVE_SOURCES_NAME}
        )
    endforeach ()
endfunction()
