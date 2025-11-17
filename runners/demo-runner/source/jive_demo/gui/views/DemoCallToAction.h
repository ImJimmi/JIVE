#pragma once

#include <jive_demo/gui/tokens/Colours.h>

namespace jive_demo::views
{
    class DemoCallToAction
    {
    public:
        [[nodiscard]] auto& withText(const juce::String& ctaText) noexcept
        {
            text = ctaText;
            return *this;
        }

        [[nodiscard]] auto& withBaseColour(const juce::Colour& colour) noexcept
        {
            baseColour = colour;
            return *this;
        }

        [[nodiscard]] auto& withHoverColour(const juce::Colour& colour) noexcept
        {
            hoverColour = colour;
            return *this;
        }

        [[nodiscard]] auto operator()() const
        {
            const auto style = [this] {
                return new jive::Object{
                    { "foreground", "#000000" },
                    { "border", juce::var{} },
                    { "border-radius", 5 },
                    {
                        "background",
                        new jive::Object{
                            {
                                "stops",
                                new jive::Object{
                                    { "0.0", jive::toVar(baseColour.brighter(0.15f)) },
                                    { "0.5", jive::toVar(baseColour) },
                                    { "1.0", jive::toVar(baseColour.darker(0.1f)) },
                                },
                            },
                            { "gradient", "linear" },
                            { "orientation", "horizontal" },
                        },
                    },
                    {
                        "hover",
                        new jive::Object{
                            {
                                "background",
                                new jive::Object{
                                    {
                                        "stops",
                                        new jive::Object{
                                            { "0.0", jive::toVar(hoverColour.brighter(0.15f)) },
                                            { "0.5", jive::toVar(hoverColour) },
                                            { "1.0", jive::toVar(hoverColour.darker(0.1f)) },
                                        },
                                    },
                                    { "gradient", "linear" },
                                    { "orientation", "horizontal" },
                                },
                            },
                        },
                    },
                };
            };

            return juce::ValueTree{
                "Button",
                {
                    { "padding", "5 10" },
                    { "margin", "0 0 10 0" },
                    { "style", style() },
                },
                {
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", text },
                            { "justification", "centred" },
                        },
                    },
                },
            };
        }

    private:
        juce::String text;
        juce::Colour baseColour = colours::blue500;
        juce::Colour hoverColour = colours::blue400;
    };
} // namespace jive_demo::views
