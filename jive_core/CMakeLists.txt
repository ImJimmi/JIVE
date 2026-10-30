add_library(jive_core
            INTERFACE
            EXCLUDE_FROM_ALL
)
add_library(jive::jive_core
            ALIAS
            jive_core
)

target_sources(jive_core
               PUBLIC algorithms/jive_Bezier.cpp
                      algorithms/jive_Bezier.h
                      algorithms/jive_Find.cpp
                      algorithms/jive_Find.h
                      algorithms/jive_Interpolate.cpp
                      algorithms/jive_Interpolate.h
                      algorithms/jive_TransferFunction.h
                      algorithms/jive_Visitor.h
                      compile-time-helpers/jive_JuceVersion.h
                      files/jive_FileObserver.h
                      files/jive_SourceFile.cpp
                      files/jive_SourceFile.h
                      files/jive_SourceRoot.cpp
                      files/jive_SourceRoot.h
                      geometry/jive_Anchor.cpp
                      geometry/jive_Anchor.h
                      geometry/jive_BorderRadii.h
                      geometry/jive_BoxModel.cpp
                      geometry/jive_BoxModel.h
                      geometry/jive_Length.cpp
                      geometry/jive_Length.h
                      geometry/jive_Orientation.cpp
                      geometry/jive_Orientation.h
                      interface/jive_InteractionState.h
                      jive_core.h
                      kinetics/jive_Easing.cpp
                      kinetics/jive_Easing.h
                      kinetics/jive_Transition.cpp
                      kinetics/jive_Transition.h
                      kinetics/jive_Transitions.cpp
                      kinetics/jive_Transitions.h
                      logging/jive_ConsoleProgressBar.cpp
                      logging/jive_ConsoleProgressBar.h
                      logging/jive_ScopeIndentedLogger.cpp
                      logging/jive_ScopeIndentedLogger.h
                      logging/jive_StringStreams.cpp
                      logging/jive_StringStreams.h
                      testing/jive_TestZip.h
                      testing/jive_UnitTest.h
                      time/jive_TimeParser.h
                      time/jive_TimeParser.h
                      time/jive_Timer.cpp
                      time/jive_Timer.h
                      values/jive_Colours.cpp
                      values/jive_Colours.h
                      values/jive_Event.cpp
                      values/jive_Event.h
                      values/jive_Object.cpp
                      values/jive_Object.h
                      values/jive_Property.cpp
                      values/jive_Property.h
                      values/jive_PropertyBehaviours.h
                      values/jive_XmlParser.cpp
                      values/jive_XmlParser.h
                      values/variant-converters/jive_AttributedStringVariantConverters.cpp
                      values/variant-converters/jive_AttributedStringVariantConverters.h
                      values/variant-converters/jive_FlexVariantConverters.cpp
                      values/variant-converters/jive_FlexVariantConverters.h
                      values/variant-converters/jive_GridVariantConverters.cpp
                      values/variant-converters/jive_GridVariantConverters.h
                      values/variant-converters/jive_MiscVariantConverters.h
                      values/variant-converters/jive_VariantConvertion.cpp
                      values/variant-converters/jive_VariantConvertion.h
)

target_include_directories(jive_core
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(jive_core
                      INTERFACE juce::juce_gui_basics
)

if (JIVE_ENABLE_PERFETTO)
    add_melatonin_perfetto(jive_core)
endif()
