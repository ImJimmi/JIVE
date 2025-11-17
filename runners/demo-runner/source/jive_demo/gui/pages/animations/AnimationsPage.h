#pragma once

#include "ComponentAnimations.h"
#include "FlexBoxAnimations.h"
#include "GridAnimations.h"
#include "StyleSheetAnimations.h"

#include <jive_demo/gui/pages/CodeExamplePage.h>
#include <jive_demo/gui/pages/PageWithNavigation.h>
#include <jive_demo/gui/views/Icons.h>
#include <jive_demo/gui/views/PageHeader.h>
#include <jive_demo/gui/views/PageLink.h>
#include <jive_demo/gui/views/Subtitle.h>
#include <jive_demo/gui/views/Title.h>

namespace jive_demo
{
    namespace views
    {
        [[nodiscard]] inline auto navButton(const juce::String& text,
                                            const juce::Identifier& id)
        {
            return juce::ValueTree{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
                            { "border-radius", 2.5 },
                            { "background", juce::var{} },
                            { "shadow", juce::var{} },
                            {
                                "hover",
                                new jive::Object{
                                    {
                                        "checked",
                                        new jive::Object{
                                            { "background", "rgba(255.0, 255.0, 255.0, 0.2)" },
                                        },
                                    },
                                    { "background", "rgba(255.0, 255.0, 255.0, 0.08)" },
                                },
                            },
                            {
                                "checked",
                                new jive::Object{
                                    { "background", "rgba(255.0, 255.0, 255.0, 0.2)" },
                                },
                            },
                        },
                    },
                    { "id", jive::toVar(id) },
                    { "padding", "5 10" },
                    { "min-width", 120 },
                    { "radio-group", 1 },
                    { "toggleable", true },
                    { "toggle-on-click", true },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", text },
                            { "justification", "centred-left" },
                        },
                    },
                },
            };
        }

        [[nodiscard]] inline auto navButtons()
        {
            return juce::ValueTree{
                "Component",
                {
                    {
                        "style",
                        new jive::Object{
                            { "background", jive::toVar(colours::menu) },
                            { "border-radius", 0 },
                        },
                    },
                    { "align-self", "stretch" },
                    { "padding", 5 },
                    { "gap", 5 },
                },
                {
                    navButton("Component", "nav-component"),
                    navButton("Flex Box", "nav-flex"),
                    navButton("Grid", "nav-grid"),
                    navButton("Style Sheets", "nav-style-sheets"),
                },
            };
        }

        [[nodiscard]] inline auto animationsPage()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "id", "animations" },
                },
                {
                    PageHeader{}
                        .withTitle("Animations")
                        .withIcon(icons::large::motionBlur())(),
                    juce::ValueTree{
                        "Component",
                        {
                            { "flex-direction", "row" },
                            { "flex-grow", 1 },
                        },
                        {
                            navButtons(),
                            juce::ValueTree{
                                "Component",
                                {
                                    { "id", "animations-page-content" },
                                    { "padding", 15 },
                                    { "flex-grow", 1 },
                                    { "align-items", "stretch" },
                                },
                            },
                        },
                    },
                }
            };
        }
    } // namespace views

    class AnimationsPagePresenter : private PageWithNavigation
    {
    public:
        explicit AnimationsPagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::animations,
            }
            , onComponentButtonClicked{
                jive::findElementWithID(view, "nav-component"),
                "on-click",
            }
            , onFlexBoxButtonClicked{
                jive::findElementWithID(view, "nav-flex"),
                "on-click",
            }
            , onGridButtonClicked{
                jive::findElementWithID(view, "nav-grid"),
                "on-click",
            }
            , onStyleSheetsButtonClicked{
                jive::findElementWithID(view, "nav-style-sheets"),
                "on-click",
            }
        {
            onComponentButtonClicked.onTrigger = [this] {
                setContent(componentTab.present());
            };
            onFlexBoxButtonClicked.onTrigger = [this] {
                setContent(flexBoxTab.present());
            };
            onGridButtonClicked.onTrigger = [this] {
                setContent(gridTab.present());
            };
            onStyleSheetsButtonClicked.onTrigger = [this] {
                setContent(styleSheetsTab.present());
            };

            setView(view);
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        void setContent(const juce::ValueTree& content)
        {
            auto container = jive::findElementWithID(view, "animations-page-content");
            container.removeAllChildren(nullptr);
            container.appendChild(content, nullptr);
        }

        juce::ValueTree view{ views::animationsPage() };

        jive::Event onComponentButtonClicked;
        jive::Event onFlexBoxButtonClicked;
        jive::Event onGridButtonClicked;
        jive::Event onStyleSheetsButtonClicked;

        ComponentAnimationsPresenter componentTab;
        FlexBoxAnimationsPresenter flexBoxTab;
        GridAnimationsPresenter gridTab;
        StyleSheetAnimationsPresenter styleSheetsTab;
    };
} // namespace jive_demo
