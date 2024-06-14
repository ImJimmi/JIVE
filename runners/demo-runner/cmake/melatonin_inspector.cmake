function(add_melatonin_inspector TARGET)
    Include(FetchContent)
    FetchContent_Declare(melatonin_inspector
        GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
        GIT_TAG origin/main
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/melatonin_inspector
    )
    FetchContent_MakeAvailable(melatonin_inspector)

    target_link_libraries(${TARGET} INTERFACE melatonin_inspector)
endfunction()
