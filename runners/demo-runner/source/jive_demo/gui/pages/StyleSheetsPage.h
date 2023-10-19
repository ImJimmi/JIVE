#pragma once

#include "CodeExamplePage.h"
#include "PageWithNavigation.h"

#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageHeader.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static auto styleSheetsBody()
        {
            static constexpr auto exampleStyleSheet = [] {
                return new jive::Object{
                    {
                        "background",
                        new jive::Object{
                            { "gradient", "linear" },
                            {
                                "stops",
                                new jive::Object{
                                    { "0.0", "#C7EDE4" },
                                    { "1.0", "#A9E4EF" },
                                },
                            },
                        },
                    },
                    { "foreground", "rgba(0, 0, 0, 0.9)" },
                    { "border", "#7A306C" },
                    { "border-radius", "5 15" },
                    { "font-weight", "bold" },
                    { "font-size", 20 },
                };
            };

            static constexpr auto exampleView = [] {
                static constexpr auto button1 = [] {
                    return juce::ValueTree{
                        "Button",
                        {
                            { "style", exampleStyleSheet() },
                            { "padding", 15 },
                            { "border-width", 2 },
                            { "margin", "0 0 15 0" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Hello, World!" },
                                },
                            },
                        },
                    };
                };
                static constexpr auto button2 = [] {
                    return juce::ValueTree{
                        "Button",
                        {
                            {
                                "style",
                                new jive::Object{
                                    { "background", "#FFBA08" },
                                    { "foreground", "#1C3144" },
                                    { "border", "#D00000" },
                                    { "font-size", 12 },
                                    { "border-radius", 5 },
                                },
                            },
                            { "padding", 15 },
                            { "border-width", 2 },
                            { "margin", "0 0 15 0" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Lorum Ipsum?" },
                                },
                            },
                        },
                    };
                };
                static constexpr auto button3 = [] {
                    return juce::ValueTree{
                        "Button",
                        {
                            {
                                "style",
                                new jive::Object{
                                    {
                                        "background",
                                        new jive::Object{
                                            { "gradient", "linear" },
                                            { "orientation", "horizontal" },
                                            {
                                                "stops",
                                                new jive::Object{
                                                    { "0.0", "#A1E8AF" },
                                                    { "1.0", "#94C595" },
                                                },
                                            },
                                        },
                                    },
                                    { "foreground", "#3A2449" },
                                    { "border", "#372772" },
                                    { "font-family", "Brush Script MT" },
                                },
                            },
                            { "padding", 15 },
                            { "border-width", 2 },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    {
                                        "text",
                                        "Don't Panic.",
                                    },
                                },
                            },
                        },
                    };
                };

                return juce::ValueTree{
                    "Component",
                    {},
                    {
                        button1(),
                        button2(),
                        button3(),
                    },
                };
            };

            return CodeExamplePage{}
                .withBodyText("JIVE's Sheet Sheets make styling components effortless by providing an API to specify "
                              "the most common graphics properties like background, foreground, borders, and much "
                              "more. Style Sheets can be used standalone, but are best used alongside JIVE's Layouts "
                              "module to completely remove the need to write any custom Component classes ever again.\n"
                              "The following JSON is used to style the \"Hello, World!\" button below:")
                .withCodeSnippet(juce::JSON::toString(juce::var{ exampleStyleSheet() }))
                .withExampleView(exampleView())()
                .setProperty("flex-grow", 1, nullptr);
        }

        [[nodiscard]] static auto styleSheetsPage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "style-sheets-page" },
                },
                {
                    PageHeader{}
                        .withTitle("Style Sheets")
                        .withIcon(icons::large::palette())(),
                    styleSheetsBody(),
                }
            };
        }
    } // namespace views

    class StyleSheetsPagePresenter : public PageWithNavigation
    {
    public:
        explicit StyleSheetsPagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::styleSheets,
            }
        {
            setView(view);
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::styleSheetsPage() };
    };
} // namespace jive_demo
