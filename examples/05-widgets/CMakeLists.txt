jive_add_example(jive-05-widgets
                 PLUGIN
                 PRODUCT_NAME "JIVE Widgets"
                 PLUGIN_CODE "Wdgt"
                 SOURCES source/PluginFilter.cpp
                         source/PluginProcessor.h
                         source/Presenter.h
                         source/SegmentedSlider.h
                         source/XYPad.h
                         source/XYPadItem.h
                 RESOURCES ../resources/rubik.ttf
                           ../resources/rubik-mono-one.ttf
                           ../resources/source-code-pro.ttf
)

jive_sources(jive-05-widgets
             SOURCES "public/*"
)
