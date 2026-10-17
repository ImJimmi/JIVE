jive_add_example(jive-02-layouts
                 APP
                 PRODUCT_NAME "JIVE Layouts"
                 SOURCES source/main.cpp
                         source/Presenter.h
                 RESOURCES ../resources/rubik.ttf
                           ../resources/rubik-mono-one.ttf
                           ../resources/source-code-pro.ttf
)

jive_sources(jive-02-layouts
             SOURCES "public/*"
)
