jive_add_example(jive-04-transitions
                 APP
                 PRODUCT_NAME "JIVE Transitions"
                 SOURCES source/main.cpp
                         source/Presenter.h
                 RESOURCES ../resources/rubik.ttf
                           ../resources/rubik-mono-one.ttf
                           ../resources/source-code-pro.ttf
)

jive_sources(jive-04-transitions
             SOURCES "public/*"
)
