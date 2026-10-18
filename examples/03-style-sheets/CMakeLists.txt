jive_add_example(jive-03-style-sheets
                 APP
                 PRODUCT_NAME "JIVE Style Sheets"
                 SOURCES source/main.cpp
                         source/Presenter.h
                 RESOURCES ../resources/rubik.ttf
                           ../resources/rubik-mono-one.ttf
                           ../resources/source-code-pro.ttf
)

jive_sources(jive-03-style-sheets
             SOURCES "public/*"
)
