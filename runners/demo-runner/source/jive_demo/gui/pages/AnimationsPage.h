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
        [[nodiscard]] inline auto navButton(const juce::String& text,
                                            const juce::Identifier& id)
        {
            return juce::ValueTree{
                "Button",
                {
                    {
                        "style",
                        new jive::Object{
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
                            { "border-radius", 2.5 },
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
                        },
                    },
                    { "align-self", "stretch" },
                    { "padding", 5 },
                    { "gap", 5 },
                },
                {
                    navButton("Box Model", "nav-box-model"),
                    navButton("Attributes", "nav-attributes"),
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

        [[nodiscard]] inline auto boxModelAnimationsDemo()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "align-items", "stretch" },
                    { "flex-grow", 1 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "flex-direction", "row" },
                            { "align-items", "centre" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Animate:" },
                                    { "margin", "0 15 0 0" },
                                },
                            },
                            DemoCallToAction{}
                                .withText("Position")()
                                .setProperty("id", "animate-position-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Size")()
                                .setProperty("id", "animate-size-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Border")()
                                .setProperty("id", "animate-border-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "display", "block" },
                            { "flex-grow", 1 },
                        },
                        {
                            juce::ValueTree{
                                "Component",
                                {
                                    { "id", "animated-box" },
                                    { "width", 75 },
                                    { "height", 75 },
                                    { "centre-x", "50%" },
                                    { "centre-y", "50%" },
                                    {
                                        "transition",
                                        "width 200ms ease-in-out, "
                                        "height 200ms ease-in-out, "
                                        "centre-x 350ms ease-in-out, "
                                        "centre-y 350ms ease-in-out, "
                                        "border-width 250ms ease-in-out",
                                    },
                                    { "border-width", 1.5 },
                                    {
                                        "style",
                                        new jive::Object{
                                            { "background", jive::toVar(colours::red500) },
                                            { "border", jive::toVar(colours::blue500) },
                                            { "border-radius", 10 },
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            };
        }
    } // namespace views

    class BoxModelTabPresenter
    {
    public:
        BoxModelTabPresenter()
            : onAnimatePositionButtonClicked{
                jive::findElementWithID(view, "animate-position-button"),
                "on-click",
            }
            , onAnimateSizeButtonClicked{
                jive::findElementWithID(view, "animate-size-button"),
                "on-click",
            }
            , onAnimateBorderButtonClicked{
                jive::findElementWithID(view, "animate-border-button"),
                "on-click",
            }
        {
            onAnimatePositionButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto cx = juce::Random::getSystemRandom().nextInt({ 15, 85 });
                const auto cy = juce::Random::getSystemRandom().nextInt({ 15, 85 });
                box.setProperty("centre-x", juce::String{ cx } + "%", nullptr);
                box.setProperty("centre-y", juce::String{ cy } + "%", nullptr);
            };
            onAnimateSizeButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto width = juce::Random::getSystemRandom().nextInt({ 30, 100 });
                box.setProperty("width", width, nullptr);
                const auto height = juce::Random::getSystemRandom().nextInt({ 30, 100 });
                box.setProperty("height", height, nullptr);
            };
            onAnimateBorderButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto thickness = juce::Random::getSystemRandom().nextFloat() * 10.0f;
                box.setProperty("border-width", thickness, nullptr);
            };
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::boxModelAnimationsDemo() };

        jive::Event onAnimatePositionButtonClicked;
        jive::Event onAnimateSizeButtonClicked;
        jive::Event onAnimateBorderButtonClicked;
    };

    class AnimationsPagePresenter : private PageWithNavigation
    {
    public:
        explicit AnimationsPagePresenter(WindowState windowState)
            : PageWithNavigation{
                windowState,
                Page::animations,
            }
            , onBoxModelButtonClicked{
                jive::findElementWithID(view, "nav-box-model"),
                "on-click",
            }
            , onAttributesButtonClicked{
                jive::findElementWithID(view, "nav-attributes"),
                "on-click",
            }
        {
            onBoxModelButtonClicked.onTrigger = [this] {
                setContent(boxModelTab.present());
            };
            onAttributesButtonClicked.onTrigger = [this] {
                setContent(juce::ValueTree{ "Text", { { "text", "FOOOOOOO!" } } });
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

        jive::Event onBoxModelButtonClicked;
        jive::Event onAttributesButtonClicked;

        BoxModelTabPresenter boxModelTab;
    };
} // namespace jive_demo
