#pragma once

#include <jive_demo/gui/views/DemoCallToAction.h>

#include <jive_layouts/jive_layouts.h>

namespace jive_demo
{
    static constexpr std::array backgroundColours{
        "#7E0000",
        "#3A4800",
        "#004575",
    };
    static constexpr std::array foregroundColours{
        "##F2902E",
        "#00CAAF",
        "#BB92FF",
    };

    [[nodiscard]] inline auto getRandomBorderRadius()
    {
        static auto& rng = juce::Random::getSystemRandom();
        return 10.0f + rng.nextFloat() * 30.0f;
    }

    [[nodiscard]] inline auto getRandomFontSize()
    {
        static auto& rng = juce::Random::getSystemRandom();
        return 15.0f + rng.nextFloat() * 15.0f;
    }

    [[nodiscard]] inline auto getRandomFontStretch()
    {
        static auto& rng = juce::Random::getSystemRandom();
        return 0.67f + rng.nextFloat() * 0.67f;
    }

    [[nodiscard]] inline auto getRandomLetterSpacing()
    {
        static auto& rng = juce::Random::getSystemRandom();
        return rng.nextFloat() * 20.0f - 10.0f;
    }

    namespace views
    {
        [[nodiscard]] inline auto styleSheetAnimationsDemo(std::size_t backgroundColourIndex,
                                                           std::size_t foregroundColourIndex,
                                                           std::size_t borderColourIndex)
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
                            { "flex-wrap", "wrap" },
                            { "margin", "0 0 20 0" },
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
                                .withText("Background")()
                                .setProperty("id", "animate-background-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Foreground")()
                                .setProperty("id", "animate-foreground-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Border Colour")()
                                .setProperty("id", "animate-border-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Border Radius")()
                                .setProperty("id", "animate-border-radius-button", nullptr)
                                .setProperty("margin", "0 10 0 0", nullptr),
                            DemoCallToAction{}
                                .withText("Font")()
                                .setProperty("id", "animate-font-button", nullptr)
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
                                "Button",
                                {
                                    { "id", "styled-box" },
                                    { "width", 250 },
                                    { "height", 200 },
                                    { "centre-x", "50%" },
                                    { "centre-y", "50%" },
                                    { "border-width", 15 },
                                    { "justify-content", "centre" },
                                    { "align-items", "centre" },
                                    {
                                        "style",
                                        new jive::Object{
                                            { "background", jive::toVar(jive::parseColour(backgroundColours[backgroundColourIndex])) },
                                            { "foreground", jive::toVar(jive::parseColour(foregroundColours[foregroundColourIndex])) },
                                            { "border", jive::toVar(jive::parseColour(foregroundColours[borderColourIndex])) },
                                            { "border-radius", getRandomBorderRadius() },
                                            { "font-family", "Rubik Mono One" },
                                            { "font-size", getRandomFontSize() },
                                            { "font-stretch", 1.0f },
                                            { "letter-spacing", 0.0f },
                                            {
                                                "transition",
                                                "background 500ms ease-in-out,"
                                                "foreground 400ms ease-in-out,"
                                                "border 500ms ease-in-out,"
                                                "border-radius 500ms ease-in-out,"
                                                "font-size 500ms ease-in-out,"
                                                "font-stretch 300ms ease-in-out,"
                                                "letter-spacing 300ms ease-in-out,",
                                            },
                                            {
                                                "hover",
                                                new jive::Object{
                                                    { "background", jive::toVar(juce::Colours::black) },
                                                    { "foreground", jive::toVar(juce::Colours::white) },
                                                    { "border", jive::toVar(juce::Colours::white) },
                                                    { "border-radius", "50 150" },
                                                    { "font-size", 30.0f },
                                                    { "font-stretch", 1.0f },
                                                    { "letter-spacing", 0.0f },
                                                },
                                            },
                                        },
                                    },
                                },
                                {
                                    juce::ValueTree{
                                        "Text",
                                        {
                                            { "text", "Hello,\nWorld!" },
                                            { "justification", "centred" },
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

    class StyleSheetAnimationsPresenter
    {
    public:
        StyleSheetAnimationsPresenter()
            : onAnimateBackgroundButtonClicked{
                jive::findElementWithID(view, "animate-background-button"),
                "on-click",
            }
            , onAnimateForegroundButtonClicked{
                jive::findElementWithID(view, "animate-foreground-button"),
                "on-click",
            }
            , onAnimateBorderButtonClicked{
                jive::findElementWithID(view, "animate-border-button"),
                "on-click",
            }
            , onAnimateBorderRadiusButtonClicked{
                jive::findElementWithID(view, "animate-border-radius-button"),
                "on-click",
            }
            , onAnimateFontButtonClicked{
                jive::findElementWithID(view, "animate-font-button"),
                "on-click",
            }
        {
            onAnimateBackgroundButtonClicked.onTrigger = [this] {
                increment(backgroundColourIndex);

                auto& style = dynamic_cast<jive::Object&>(*jive::findElementWithID(view, "styled-box")["style"].getDynamicObject());
                style.setProperty("background", jive::toVar(jive::parseColour(backgroundColours[backgroundColourIndex])));
            };
            onAnimateForegroundButtonClicked.onTrigger = [this] {
                increment(foregroundColourIndex);

                auto& style = dynamic_cast<jive::Object&>(*jive::findElementWithID(view, "styled-box")["style"].getDynamicObject());
                style.setProperty("foreground", jive::toVar(jive::parseColour(foregroundColours[foregroundColourIndex])));
            };
            onAnimateBorderButtonClicked.onTrigger = [this] {
                increment(borderColourIndex);

                auto& style = dynamic_cast<jive::Object&>(*jive::findElementWithID(view, "styled-box")["style"].getDynamicObject());
                style.setProperty("border", jive::toVar(jive::parseColour(foregroundColours[borderColourIndex])));
            };
            onAnimateBorderRadiusButtonClicked.onTrigger = [this] {
                auto& style = dynamic_cast<jive::Object&>(*jive::findElementWithID(view, "styled-box")["style"].getDynamicObject());
                style.setProperty("border-radius", getRandomBorderRadius());
            };
            onAnimateFontButtonClicked.onTrigger = [this] {
                auto& style = dynamic_cast<jive::Object&>(*jive::findElementWithID(view, "styled-box")["style"].getDynamicObject());
                style.setProperty("font-size", getRandomFontSize());
                style.setProperty("font-stretch", getRandomFontStretch());
                style.setProperty("letter-spacing", getRandomLetterSpacing());
            };
        }

        [[nodiscard]] auto present() const
        {
            return view;
        }

    private:
        void increment(std::size_t& colourIndex)
        {
            if (++colourIndex >= 3)
                colourIndex = 0;
        }

        juce::ValueTree view{ views::styleSheetAnimationsDemo(0, 0, 0) };

        jive::Event onAnimateBackgroundButtonClicked;
        jive::Event onAnimateForegroundButtonClicked;
        jive::Event onAnimateBorderButtonClicked;
        jive::Event onAnimateBorderRadiusButtonClicked;
        jive::Event onAnimateFontButtonClicked;

        std::size_t backgroundColourIndex = 0;
        std::size_t foregroundColourIndex = 0;
        std::size_t borderColourIndex = 0;
    };
} // namespace jive_demo
