#pragma once

#include <jive_demo/gui/WindowState.h>
#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageLink.h>
#include <jive_demo/gui/views/Subtitle.h>
#include <jive_demo/gui/views/Title.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static auto welcome()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "align-items", "centre" },
                    { "margin", "0 0 50 0" },
                },
                {
                    Title{}
                        .withText("Welcome to JIVE!")
                        .withColour(colours::blue400)()
                        .setProperty("id", "title", nullptr),
                    Subtitle{}
                        .withText("The ultimate JUCE extension for building GUIs")()
                        .setProperty("id", "subtitle", nullptr),
                },
            };
        }

        class PageButton
        {
        public:
            [[nodiscard]] auto& forPage(Page forPage)
            {
                page = forPage;
                return *this;
            }

            [[nodiscard]] auto operator()() const
            {
                juce::ValueTree icon;
                juce::String name;

                switch (page)
                {
                case Page::animations:
                    icon = icons::large::motionBlur();
                    name = "Animations";
                    break;
                case Page::development:
                    icon = icons::large::code();
                    name = "Development";
                    break;
                case Page::layouts:
                    icon = icons::large::layout();
                    name = "Layouts";
                    break;
                case Page::styleSheets:
                    icon = icons::large::palette();
                    name = "Style Sheets";
                    break;
                case Page::home:
                    [[fallthrough]];
                case Page::numPages:
                    [[fallthrough]];
                default:
                    break;
                }

                static constexpr auto style = [] {
                    return new jive::Object{
                        {
                            "background",
                            juce::String{ "linear-gradient(#{start}, #{end})" }
                                .replace("{start}", colours::widgetStart.toDisplayString(false))
                                .replace("{end}", colours::widgetEnd.toDisplayString(false)),
                        },
                        { "border", "black" },
                        { "border-radius", 5 },
                        {
                            "hover",
                            new jive::Object{
                                {
                                    "background",
                                    juce::String{ "linear-gradient(#{start}, #{end})" }
                                        .replace("{start}", colours::widgetStart.brighter(0.02f).toDisplayString(false))
                                        .replace("{end}", colours::widgetEnd.brighter(0.01f).toDisplayString(false)),
                                },
                            },
                        },
                    };
                };

                return juce::ValueTree{
                    "Button",
                    {
                        { "flex-direction", "column" },
                        { "align-items", "centre" },
                        { "border-width", 1.5 },
                        { "padding", 20 },
                        { "style", style() },
                    },
                    {
                        icon
                            .setProperty("margin", "0 0 15 0", nullptr)
                            .setProperty("style",
                                         new jive::Object{
                                             { "fill", jive::toVar(colours::blue400) },
                                         },
                                         nullptr),
                        Subtitle{}
                            .withText(name)(),
                    },
                };
            }

        private:
            Page page;
        };

        [[nodiscard]] static auto navigation()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "main-nav" },
                    { "display", "grid" },
                    { "grid-template-rows", "1fr 1fr" },
                    { "grid-template-columns", "1fr 1fr" },
                    { "gap", 25 },
                },
                {
                    PageButton{}
                        .forPage(Page::layouts)()
                        .setProperty("id", "layouts", nullptr),
                    PageButton{}
                        .forPage(Page::styleSheets)()
                        .setProperty("id", "style-sheets", nullptr),
                    PageButton{}
                        .forPage(Page::development)()
                        .setProperty("id", "development", nullptr),
                    PageButton{}
                        .forPage(Page::animations)()
                        .setProperty("id", "animations", nullptr),
                },
            };
        }

        [[nodiscard]] static auto homePage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "home-page" },
                    { "align-items", "centre" },
                    { "justify-content", "centre" },
                },
                {
                    welcome(),
                    navigation(),
                },
            };
        }
    } // namespace views

    class HomePagePresenter
    {
    public:
        explicit HomePagePresenter(WindowState windowState)
            : view{ views::homePage() }
            , onLayoutsButtonClicked{
                jive::find(view, [](const auto& element) {
                    return element["id"].toString() == "layouts";
                }),
                "on-click",
            }
            , onStyleSheetsButtonClicked{
                jive::find(view, [](const auto& element) {
                    return element["id"].toString() == "style-sheets";
                }),
                "on-click",
            }
            , onDevelopmentButtonClicked{
                jive::find(view, [](const auto& element) {
                    return element["id"].toString() == "development";
                }),
                "on-click",
            }
            , onAnimationsButtonClicked{
                jive::find(view, [](const auto& element) {
                    return element["id"].toString() == "animations";
                }),
                "on-click",
            }
        {
            onLayoutsButtonClicked.onTrigger = [windowState]() mutable {
                windowState.setPage(Page::layouts);
            };
            onStyleSheetsButtonClicked.onTrigger = [windowState]() mutable {
                windowState.setPage(Page::styleSheets);
            };
            onDevelopmentButtonClicked.onTrigger = [windowState]() mutable {
                windowState.setPage(Page::development);
            };
            onAnimationsButtonClicked.onTrigger = [windowState]() mutable {
                windowState.setPage(Page::animations);
            };
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view;

        jive::Event onLayoutsButtonClicked;
        jive::Event onStyleSheetsButtonClicked;
        jive::Event onDevelopmentButtonClicked;
        jive::Event onAnimationsButtonClicked;
    };
} // namespace jive_demo
