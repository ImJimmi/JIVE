#pragma once

#include <jive_demo/gui/views/DemoCallToAction.h>

namespace jive_demo
{
    [[nodiscard]] inline auto calculateRandomOpacity()
    {
        return 0.2f + juce::Random::getSystemRandom().nextFloat() * 0.8f;
    }

    namespace views
    {
        [[nodiscard]] inline auto animatedBox()
        {
            return juce::ValueTree{
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
                        "border-width 250ms ease-in-out, "
                        "opacity 500ms ease-in-out",
                    },
                    { "border-width", 1.5 },
                    {
                        "style",
                        new jive::Object{
                            { "background", jive::toVar(colours::blue500) },
                            { "border", jive::toVar(colours::red500) },
                            { "border-radius", 10 },
                        },
                    },
                },
            };
        }

        [[nodiscard]] inline auto componentAnimationsDemo()
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
                            DemoCallToAction{}
                                .withText("Opacity")()
                                .setProperty("id", "animate-opacity-button", nullptr)
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
                            animatedBox(),
                        },
                    },
                },
            };
        }
    } // namespace views

    class ComponentAnimationsPresenter
    {
    public:
        ComponentAnimationsPresenter()
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
            , onAnimateOpacityButtonClicked{
                jive::findElementWithID(view, "animate-opacity-button"),
                "on-click",
            }
        {
            onAnimatePositionButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto cx = juce::Random::getSystemRandom().nextInt({ 20, 80 });
                const auto cy = juce::Random::getSystemRandom().nextInt({ 20, 80 });
                box.setProperty("centre-x", juce::String{ cx } + "%", nullptr);
                box.setProperty("centre-y", juce::String{ cy } + "%", nullptr);
            };
            onAnimateSizeButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto width = juce::Random::getSystemRandom().nextInt({ 30, 120 });
                box.setProperty("width", width, nullptr);
                const auto height = juce::Random::getSystemRandom().nextInt({ 30, 120 });
                box.setProperty("height", height, nullptr);
            };
            onAnimateBorderButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                const auto thickness = 1.0f + juce::Random::getSystemRandom().nextFloat() * 14.0f;
                box.setProperty("border-width", thickness, nullptr);
            };
            onAnimateOpacityButtonClicked.onTrigger = [this] {
                auto box = jive::findElementWithID(view, "animated-box");
                box.setProperty("opacity", calculateRandomOpacity(), nullptr);
            };
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::componentAnimationsDemo() };

        jive::Event onAnimatePositionButtonClicked;
        jive::Event onAnimateSizeButtonClicked;
        jive::Event onAnimateBorderButtonClicked;
        jive::Event onAnimateOpacityButtonClicked;
    };
} // namespace jive_demo
