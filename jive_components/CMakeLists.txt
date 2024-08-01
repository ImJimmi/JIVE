add_library(jive_components
            INTERFACE
            EXCLUDE_FROM_ALL
)
add_library(jive::jive_components
            ALIAS
            jive_components
)

target_sources(jive_components
               PUBLIC accessibility/jive_IgnoredComponent.cpp
                      accessibility/jive_IgnoredComponent.h
                      canvases/jive_BackgroundCanvas.cpp
                      canvases/jive_BackgroundCanvas.h
                      canvases/jive_Canvas.cpp
                      canvases/jive_Canvas.h
                      containers/jive_DocumentWindow.cpp
                      containers/jive_DocumentWindow.h
                      utilities/jive_HierarchyTraversal.h
                      widgets/jive_NormalisedProgressBar.cpp
                      widgets/jive_NormalisedProgressBar.h
                      widgets/jive_TextComponent.cpp
                      widgets/jive_TextComponent.h
                      jive_components.h
)

target_include_directories(jive_components
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(jive_components
                      INTERFACE jive::jive_core
)
