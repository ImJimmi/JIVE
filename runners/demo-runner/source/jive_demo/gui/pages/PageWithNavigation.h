#pragma once

#include <jive_demo/gui/WindowState.h>

namespace jive_demo
{
    class PageWithNavigation
    {
    public:
        PageWithNavigation(WindowState windowState, Page thisPage)
            : state{ windowState }
            , page{ thisPage }
        {
        }

        void setView(const juce::ValueTree& view)
        {
            static constexpr auto findNavButton = [](const juce::ValueTree& toSearch,
                                                     const juce::String& id) {
                return jive::find(toSearch, [&id](const auto& element) {
                    return element["id"].toString() == id;
                });
            };

            onBackButtonClicked = std::make_unique<jive::Event>(findNavButton(view, "back"),
                                                                "on-click");
            onBackButtonClicked->onTrigger = [this]() {
                state.setPage(previous(page));
            };

            onNextButtonClicked = std::make_unique<jive::Event>(findNavButton(view, "next"),
                                                                "on-click");
            onNextButtonClicked->onTrigger = [this]() {
                state.setPage(next(page));
            };
        }

    private:
        WindowState state;
        const Page page;

        std::unique_ptr<jive::Event> onBackButtonClicked;
        std::unique_ptr<jive::Event> onNextButtonClicked;
    };
} // namespace jive_demo
