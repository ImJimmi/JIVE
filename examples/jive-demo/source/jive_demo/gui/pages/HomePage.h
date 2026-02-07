#pragma once

#include <jive_demo/gui/WindowState.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    class HomePage : public jive::View
    {
    public:
        explicit HomePage(WindowState winState)
            : windowState{ winState }
        {
        }

    protected:
        juce::ValueTree initialise() final
        {
            return {
                "Component",
                {
                    { "source", "jive_demo/gui/pages/home-page/HomePage.xml" },
                },
            };
        }

        void setup(jive::GuiItem& item)
        {
            onLayoutsButtonClicked = std::make_unique<jive::Event>(
                jive::find(item.state,
                           [](const auto& element) {
                               return element["id"].toString() == "layouts";
                           }),
                "on-click",
                [this]() mutable {
                    windowState.setPage(Page::layouts);
                });

            onStyleSheetsButtonClicked = std::make_unique<jive::Event>(
                jive::find(item.state,
                           [](const auto& element) {
                               return element["id"].toString() == "style-sheets";
                           }),
                "on-click",
                [this]() mutable {
                    windowState.setPage(Page::styleSheets);
                });

            onDevelopmentButtonClicked = std::make_unique<jive::Event>(
                jive::find(item.state,
                           [](const auto& element) {
                               return element["id"].toString() == "development";
                           }),
                "on-click",
                [this]() mutable {
                    windowState.setPage(Page::development);
                });

            onAnimationsButtonClicked = std::make_unique<jive::Event>(
                jive::find(item.state,
                           [](const auto& element) {
                               return element["id"].toString() == "animations";
                           }),
                "on-click",
                [this]() mutable {
                    windowState.setPage(Page::animations);
                });
        }

    private:
        WindowState windowState;

        std::unique_ptr<jive::Event> onLayoutsButtonClicked;
        std::unique_ptr<jive::Event> onStyleSheetsButtonClicked;
        std::unique_ptr<jive::Event> onDevelopmentButtonClicked;
        std::unique_ptr<jive::Event> onAnimationsButtonClicked;
    };
} // namespace jive_demo
