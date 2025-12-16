Include(FetchContent)

function(add_melatonin_inspector TARGET)
    FetchContent_Declare(melatonin_inspector
        GIT_REPOSITORY https://github.com/sudara/melatonin_inspector.git
        GIT_TAG origin/main
        SOURCE_DIR ${CMAKE_BINARY_DIR}/melatonin_inspector/melatonin_inspector
    )
    FetchContent_MakeAvailable(melatonin_inspector)
endfunction()
