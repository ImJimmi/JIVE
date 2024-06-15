#pragma once

#include <jive_demo/gui/views/DemoCallToAction.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    namespace views
    {
        class AnimatedFlexItem : public jive::View
        {
        public:
            AnimatedFlexItem(const juce::String& propertyToAnimate, juce::Colour baseColour)
                : property{ propertyToAnimate }
                , colour{ baseColour }
            {
            }

        protected:
            juce::ValueTree initialise() final
            {
                const auto initialFlexGrow = property == "flex-grow" ? rng.nextInt({ 1, 9 }) : 1;
                const auto initialFlexShrink = property == "flex-shrink" ? rng.nextInt({ 1, 9 }) : 0;
                const auto initialFlexBasis = property == "flex-shrink" ? 200 : 0;

                return juce::ValueTree{
                    "Button",
                    {
                        { "flex-grow", initialFlexGrow },
                        { "flex-shrink", initialFlexShrink },
                        { "flex-basis", initialFlexBasis },
                        { "height", 50 },
                        { "margin", 5 },
                        { "align-items", "centre" },
                        { "justify-content", "centre" },
                        { "border-width", 1.5 },
                        {
                            "transition",
                            "flex-grow 200ms ease-in-out,"
                            "flex-shrink 200ms ease-in-out",
                        },
                        {
                            "style",
                            new jive::Object{
                                { "background", jive::toVar(colour) },
                                { "border", jive::toVar(colour.brighter(0.3f)) },
                                { "border-radius", 10 },
                                {
                                    "hover",
                                    new jive::Object{
                                        { "background", jive::toVar(colour.brighter(0.15f)) },
                                    },
                                },
                            },
                        },
                    },
                    {
                        juce::ValueTree{
                            "Text",
                            {
                                {
                                    "text",
                                    property + ": " + (property == "flex-grow" ? juce::String{ initialFlexGrow } : juce::String{ initialFlexShrink }),
                                },
                                {
                                    "style",
                                    new jive::Object{
                                        { "font-family", "Source Code Pro" },
                                        { "font-size", 10 },
                                    },
                                },
                            },
                        },
                    },
                };
            }

            void setup(jive::GuiItem&) final
            {
                animatedProperty = std::make_unique<jive::Property<float>>(getState(), property);
                animatedProperty->onValueChange = [this] {
                    getState()
                        .getChildWithName("Text")
                        .setProperty("text",
                                     property + ": " + juce::String{ *animatedProperty },
                                     nullptr);
                };

                onClick = std::make_unique<jive::Event>(getState(), "on-click");
                onClick->onTrigger = [this] {
                    if (animatedProperty != nullptr)
                    {
                        auto newValue = animatedProperty->get();

                        while (juce::approximatelyEqual(newValue, animatedProperty->get()))
                            newValue = rng.nextInt({ 1, 9 });

                        *animatedProperty = newValue;
                    }
                };
            }

        private:
            static inline juce::Random& rng = juce::Random::getSystemRandom();
            const juce::String property;
            const juce::Colour colour;

            std::unique_ptr<jive::Property<float>> animatedProperty;
            std::unique_ptr<jive::Event> onClick;
        };

        [[nodiscard]] inline auto flexBoxAnimationsDemo()
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
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Click an item to randomly change its " },
                                },
                                {
                                    juce::ValueTree{
                                        "Text",
                                        {
                                            { "text", "flex-grow" },
                                            {
                                                "style",
                                                new jive::Object{
                                                    { "font-family", "Source Code Pro" },
                                                },
                                            },
                                        },
                                    },
                                    juce::ValueTree{
                                        "Text",
                                        {
                                            { "text", " property:" },
                                        },
                                    },
                                },
                            },
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "display", "flex" },
                            { "flex-direction", "row" },
                            { "flex-grow", 1 },
                            { "padding", "20 10 10 10" },
                        },
                        {
                            jive::makeView<AnimatedFlexItem>("flex-grow", colours::blue600),
                            jive::makeView<AnimatedFlexItem>("flex-grow", colours::blue600),
                            jive::makeView<AnimatedFlexItem>("flex-grow", colours::blue600),
                            jive::makeView<AnimatedFlexItem>("flex-grow", colours::blue600),
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "flex-direction", "row" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", "Click an item to randomly change its " },
                                },
                                {
                                    juce::ValueTree{
                                        "Text",
                                        {
                                            { "text", "flex-shrink" },
                                            {
                                                "style",
                                                new jive::Object{
                                                    { "font-family", "Source Code Pro" },
                                                },
                                            },
                                        },
                                    },
                                    juce::ValueTree{
                                        "Text",
                                        {
                                            { "text", " property:" },
                                        },
                                    },
                                },
                            },
                        },
                    },
                    juce::ValueTree{
                        "Component",
                        {
                            { "display", "flex" },
                            { "flex-direction", "row" },
                            { "flex-grow", 1 },
                            { "padding", "20 10 10 10" },
                        },
                        {
                            jive::makeView<AnimatedFlexItem>("flex-shrink", colours::red600),
                            jive::makeView<AnimatedFlexItem>("flex-shrink", colours::red600),
                            jive::makeView<AnimatedFlexItem>("flex-shrink", colours::red600),
                            jive::makeView<AnimatedFlexItem>("flex-shrink", colours::red600),
                            jive::makeView<AnimatedFlexItem>("flex-shrink", colours::red600),
                        },
                    },
                },
            };
        }
    } // namespace views

    class FlexBoxAnimationsPresenter
    {
    public:
        FlexBoxAnimationsPresenter()
        {
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        juce::ValueTree view{ views::flexBoxAnimationsDemo() };
    };
} // namespace jive_demo
