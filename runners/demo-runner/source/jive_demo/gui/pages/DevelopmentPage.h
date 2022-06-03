#pragma once

#include "CodeExamplePage.h"
#include "PageWithNavigation.h"

#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageHeader.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static auto developmentBody()
        {
            static constexpr auto jiveSnippet =
                R"(auto button(auto&& props) {
    return juce::ValueTree {
        "Button",
        {
            { "padding", "5 10" },
        },
        {
            juce::ValueTree {
                "Text",
                {
                    { "text", props.text },
                },
            },
        },
    };
}
)";

            static constexpr auto reactSnippet =
                R"(function button(props) {
    return (
        <button style="padding: 5 10;">
            {props.text}
        </button>
    );
}
)";

            return CodeExamplePage{}
                .withBodyText("JIVE's #1 priority is on improving the Developer Experience of developing GUIs in "
                              "JUCE.\n"
                              "The recommended approach, as demonstrated in the source for this demo, is to use "
                              "functions to return Value Trees. This should be very familiar if you've ever used a "
                              "JavaScript framework like React, where you write functions that return HTML.\n"
                              "Below is a comparison between JIVE and its counterpart in React.")
                .withCodeSnippet(jiveSnippet)
                .withComparisonSnippet(reactSnippet)()
                .setProperty("flex-grow", 1, nullptr);
        }

        [[nodiscard]] static auto developmentPage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "style-sheets-page" },
                },
                {
                    PageHeader{}
                        .withTitle("Development")
                        .withIcon(icons::large::code())(),
                    developmentBody(),
                }
            };
        }
    } // namespace views

    class DevelopmentPagePresenter : public PageWithNavigation
    {
    public:
        explicit DevelopmentPagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::development,
            }
        {
            setView(view);
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::developmentPage() };
    };
} // namespace jive_demo
