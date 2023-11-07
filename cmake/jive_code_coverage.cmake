include_guard()

add_library(jive_code_coverage INTERFACE)
add_library(jive::code_coverage ALIAS jive_code_coverage)

if (APPLE AND JIVE_ENABLE_COVERAGE)
    target_compile_options(jive_code_coverage
    INTERFACE
        --coverage
    )

    target_link_options(jive_code_coverage
    INTERFACE
        --coverage
    )
endif()
