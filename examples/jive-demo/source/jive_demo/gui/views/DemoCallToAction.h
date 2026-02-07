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
            return juce::ValueTree{
                "Button",
                {
                    { "padding", "5 10" },
                    { "margin", "0 0 10 0" },
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
