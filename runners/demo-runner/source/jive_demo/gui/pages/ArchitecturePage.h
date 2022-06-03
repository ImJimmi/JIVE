#pragma once

#include "PageWithNavigation.h"

#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageHeader.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] static auto architectureBody()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "flex-grow", 1 },
                    { "padding", 25 },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            {
                                "text",
                                "Typical JUCE projects have very little separation between the presentation layer and "
                                "the so called Business Rules - the logic that actually implements the features of the "
                                "project. Usually you'd write Component classes that instantiate and position widgets, "
                                "as well as responding to user interactions with those widgets and manipulating the "
                                "application's state accordingly.\n"
                                "Since JIVE provides a high-level, declarative abstraction of the presentation/view "
                                "layer, it naturally encourages the separation of this layer from the service layer "
                                "(Business Rules). There are many such architectures you could use, but JIVE is "
                                "particularly suited to the Model-View-Presenter (MVP) architecture, as demonstrated "
                                "by this demo project.\n"
                                "The MVP architecture uses a \"Model\" layer to encapsulate the state of the "
                                "application, a \"View\" layer to encapsulate the presentation of the current state to "
                                "the user, and then a \"Presenter\" layer to mediate between the two. This demo uses "
                                "various Model classes for the app's state, JUCE Value Trees to describe the View "
                                "(interpreted by JIVE), and then various Presenter classes to manage the creation of "
                                "the View according to the current state, and the manipulation of the View and Model "
                                "layers according to changes in both.",
                            },
                        },
                    },
                },
            };
        }

        [[nodiscard]] static auto architecturePage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "style-sheets-page" },
                },
                {
                    PageHeader{}
                        .withTitle("Architecture")
                        .withIcon(icons::large::build())(),
                    architectureBody(),
                }
            };
        }
    } // namespace views

    class ArchitecturePagePresenter : private PageWithNavigation
    {
    public:
        explicit ArchitecturePagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::architecture,
            }
        {
            setView(view);
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::architecturePage() };
    };
} // namespace jive_demo
