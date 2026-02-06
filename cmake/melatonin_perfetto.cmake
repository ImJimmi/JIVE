include(FetchContent)

function(add_melatonin_perfetto TARGET)
    FetchContent_Declare(melatonin_perfetto
        GIT_REPOSITORY https://github.com/sudara/melatonin_perfetto.git
        GIT_TAG origin/main
    )

    FetchContent_MakeAvailable(melatonin_perfetto)

    target_link_libraries(${TARGET} INTERFACE Melatonin::Perfetto)
    target_compile_definitions(${TARGET} INTERFACE PERFETTO=1)
endfunction()
