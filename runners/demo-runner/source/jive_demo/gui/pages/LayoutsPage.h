#pragma once

#include "CodeExamplePage.h"
#include "PageWithNavigation.h"

#include <jive_demo/gui/views/DemoCallToAction.h>
#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageHeader.h>
#include <jive_demo/gui/views/PageLink.h>
#include <jive_demo/gui/views/Subtitle.h>
#include <jive_demo/gui/views/Title.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static juce::ValueTree withoutNonprintableCharacters(const juce::ValueTree& tree)
        {
            auto result = tree.createCopy();

            result.removeProperty("style", nullptr);

            for (auto child : result)
                child.copyPropertiesAndChildrenFrom(withoutNonprintableCharacters(child), nullptr);

            return result;
        }

        [[nodiscard]] static auto layoutsBody()
        {
            static constexpr auto exampleView = [] {
                return juce::ValueTree{
                    "Component",
                    {
                        { "align-items", "centre" },
                        { "padding", 20 },
                    },
                    {
                        juce::ValueTree{
                            "Text",
                            {
                                { "text", "Are you enjoying the JIVE demo?" },
                                { "margin", "0 0 10 0" },
                            },
                        },
                        DemoCallToAction{}
                            .withText("Yes")(),
                        DemoCallToAction{}
                            .withText("Yes but in red")
                            .withBaseColour(colours::red500)
                            .withHoverColour(colours::red400)(),
                    },
                };
            };

            auto format = juce::XmlElement::TextFormat{}.withoutHeader();
            format.lineWrapLength = 20;

            return CodeExamplePage{}
                .withBodyText("JIVE provides a declarative API for building UIs using XML-like data structures.\n"
                              "Common properties like enablement state, cursor style, and alt-text, as well as "
                              "popular layout patterns such as Flex and Grid are all easily configurable by "
                              "assigning attributes to value-trees.\n"
                              "For example, the XML below results in the UI to its right:")
                .withCodeSnippet(withoutNonprintableCharacters(exampleView()).toXmlString(format))
                .withExampleView(exampleView())()
                .setProperty("flex-grow", 1, nullptr);
        }

        [[nodiscard]] static auto layoutsPage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "layouts-page" },
                },
                {
                    PageHeader{}
                        .withTitle("Layouts")
                        .withIcon(icons::large::layout())(),
                    layoutsBody(),
                }
            };
        }
    } // namespace views

    class LayoutsPagePresenter : private PageWithNavigation
    {
    public:
        explicit LayoutsPagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::layouts,
            }
        {
            setView(view);
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::layoutsPage() };
    };
} // namespace jive_demo
