#pragma once

#include "WindowState.h"

#include <jive_demo/gui/pages/ArchitecturePage.h>
#include <jive_demo/gui/pages/DevelopmentPage.h>
#include <jive_demo/gui/pages/HomePage.h>
#include <jive_demo/gui/pages/LayoutsPage.h>
#include <jive_demo/gui/pages/StyleSheetsPage.h>
#include <jive_demo/gui/tokens/Colours.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static auto window()
        {
            static constexpr auto style = [] {
                return new jive::Object{
                    { "background", jive::toVar(colours::page) },
                    { "foreground", jive::toVar(colours::content) },
                    { "font-family", "Rubik" },
                    { "font-size", 14 },
                };
            };

            return juce::ValueTree{
                "Window",
                {
                    { "width", 800 },
                    { "height", 500 },
                    { "display", "grid" },
                    { "grid-template-rows", "1fr" },
                    { "grid-template-columns", "1fr" },
                    { "style", style() },
                },
            };
        }
    } // namespace views

    class WindowPresenter : private WindowState::Listener
    {
    public:
        explicit WindowPresenter(WindowState sourceState)
            : state{ sourceState }
            , window{ views::window() }
            , homePage{ sourceState }
            , layoutsPage{ sourceState }
            , styleSheetsPage{ sourceState }
            , developmentPage{ sourceState }
            , architecturePage{ sourceState }
        {
            window.appendChild(getView(state.getPage()), nullptr);
            state.addListener(*this);
        }

        juce::ValueTree present()
        {
            return window;
        }

    private:
        void pageChanged(WindowState&) final
        {
            window.removeChild(0, nullptr);
            window.appendChild(getView(state.getPage()), nullptr);
        }

        [[nodiscard]] juce::ValueTree getView(Page page)
        {
            switch (page)
            {
            case Page::home:
                return homePage.present();
            case Page::layouts:
                return layoutsPage.present();
            case Page::styleSheets:
                return styleSheetsPage.present();
            case Page::development:
                return developmentPage.present();
            case Page::architecture:
                return architecturePage.present();
            case Page::numPages:
                [[fallthrough]];
            default:
                return juce::ValueTree{
                    "Text",
                    {
                        { "text", "404" },
                    },
                };
            }
        }

        WindowState state;
        juce::ValueTree window;

        HomePagePresenter homePage;
        LayoutsPagePresenter layoutsPage;
        StyleSheetsPagePresenter styleSheetsPage;
        DevelopmentPagePresenter developmentPage;
        ArchitecturePagePresenter architecturePage;
    };
} // namespace jive_demo
