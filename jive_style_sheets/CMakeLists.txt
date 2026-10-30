add_library(jive_style_sheets
            INTERFACE
            EXCLUDE_FROM_ALL
)
add_library(jive::jive_style_sheets
            ALIAS
            jive_style_sheets
)

target_sources(jive_style_sheets
               PUBLIC components/accessibility/jive_IgnoredComponent.cpp
                      components/accessibility/jive_IgnoredComponent.h
                      components/canvases/jive_BackgroundCanvas.cpp
                      components/canvases/jive_BackgroundCanvas.h
                      components/canvases/jive_Canvas.cpp
                      components/canvases/jive_Canvas.h
                      components/containers/jive_DocumentWindow.cpp
                      components/containers/jive_DocumentWindow.h
                      components/utilities/jive_HierarchyTraversal.h
                      components/utilities/jive_PaintBeforeShowing.cpp
                      components/utilities/jive_PaintBeforeShowing.h
                      components/widgets/jive_ComboBoxComponent.cpp
                      components/widgets/jive_ComboBoxComponent.h
                      components/widgets/jive_ImageComponent.cpp
                      components/widgets/jive_ImageComponent.h
                      components/widgets/jive_NormalisedProgressBar.cpp
                      components/widgets/jive_NormalisedProgressBar.h
                      components/widgets/jive_TextComponent.cpp
                      components/widgets/jive_TextComponent.h
                      geometry/jive_BorderRadius.cpp
                      geometry/jive_BorderRadius.h
                      geometry/jive_BorderWidth.cpp
                      geometry/jive_BorderWidth.h
                      geometry/jive_Geometry.cpp
                      geometry/jive_Geometry.h
                      graphics/default-styles/jive_DefaultComboBoxStyles.h
                      graphics/default-styles/jive_DefaultComponentStyles.h
                      graphics/default-styles/jive_DefaultGroupComponentStyles.h
                      graphics/default-styles/jive_DefaultLabelStyles.h
                      graphics/default-styles/jive_DefaultProgressBarStyles.h
                      graphics/default-styles/jive_DefaultScrollBarStyles.h
                      graphics/default-styles/jive_DefaultSliderStyles.h
                      graphics/jive_ComponentTypePredicates.cpp
                      graphics/jive_ComponentTypePredicates.h
                      graphics/jive_CurrentColour.h
                      graphics/jive_Fill.cpp
                      graphics/jive_Fill.h
                      graphics/jive_FontUtilities.cpp
                      graphics/jive_FontUtilities.h
                      graphics/jive_Gradient.cpp
                      graphics/jive_Gradient.h
                      graphics/jive_Icon.cpp
                      graphics/jive_Icon.h
                      graphics/jive_LookAndFeel.cpp
                      graphics/jive_LookAndFeel.h
                      graphics/jive_Shadow.cpp
                      graphics/jive_Shadow.h
                      graphics/jive_StyleSelector.cpp
                      graphics/jive_StyleSelector.h
                      graphics/jive_Styles.h
                      graphics/themes/jive_HighContrastTheme.h
                      graphics/themes/jive_MaterialTheme.h
                      graphics/themes/jive_SteelTheme.h
                      graphics/themes/jive_SynthwaveTheme.h
                      jive_StyleSheet.cpp
                      jive_StyleSheet.h
                      jive_style_sheets.h
)

target_include_directories(jive_style_sheets
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(jive_style_sheets
                      INTERFACE jive::jive_core
)

if (JIVE_ENABLE_PERFETTO)
    add_melatonin_perfetto(jive_style_sheets)
endif()
