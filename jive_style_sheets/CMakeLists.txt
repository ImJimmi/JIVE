add_library(jive_style_sheets
            INTERFACE
            EXCLUDE_FROM_ALL
)
add_library(jive::jive_style_sheets
            ALIAS
            jive_style_sheets
)

target_sources(jive_style_sheets
               PUBLIC style-sheets/jive_StyleIdentifier.h
                      style-sheets/jive_StyleSelectors.h
                      style-sheets/jive_StyleSheet.cpp
                      style-sheets/jive_StyleSheet.h
                      jive_style_sheets.h
)

target_include_directories(jive_style_sheets
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(jive_style_sheets
                      INTERFACE jive::jive_components
)
