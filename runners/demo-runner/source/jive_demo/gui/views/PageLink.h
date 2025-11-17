#pragma once

#include <jive_demo/gui/tokens/Colours.h>

namespace jive_demo::views
{
    class PageLink
    {
    public:
        [[nodiscard]] auto& withText(const juce::String& linkText) noexcept
        {
            text = linkText;
            return *this;
        }

        [[nodiscard]] auto& withIcon(const juce::ValueTree& linkIcon) noexcept
        {
            icon = linkIcon;
            return *this;
        }

        [[nodiscard]] auto& withColour(juce::Colour foregroundColour) noexcept
        {
            colour = foregroundColour;
            return *this;
        }

        [[nodiscard]] auto& withIconOnRight() noexcept
        {
            iconOnRight = true;
            return *this;
        }

        [[nodiscard]] auto operator()() const
        {
            const auto style = [this] {
                return new jive::Object{
                    { "background", juce::var{} },
                    { "border", juce::var{} },
                    { "fill", jive::toVar(colour) },
                    { "foreground", jive::toVar(colour) },
                    { "font-size", 15 },
                    { "shadow", juce::var{} },
                    {
                        "hover",
                        new jive::Object{
                            { "fill", "white" },
                            { "foreground", "white" },
                        },
                    },
                    {
                        "active",
                        new jive::Object{
                            { "fill", jive::toVar(colour.darker(0.2f)) },
                            { "foreground", jive::toVar(colour.darker(0.2f)) },
                        },
                    },
                };
            };

            return juce::ValueTree{
                "Button",
                {
                    { "flex-direction", "row" },
                    { "padding", 0 },
                    { "justify-content", "centre" },
                    { "align-items", "centre" },
                    { "margin", "0 0 5 0" },
                    { "style", style() },
                },
                {
                    iconOnRight ? juce::ValueTree{} : icon,
                    juce::ValueTree{
                        "Text",
                        {
                            { "text", text },
                            { "margin", "0 10" },
                        },
                    },
                    iconOnRight ? icon : juce::ValueTree{},
                },
            };
        }

    private:
        juce::String text;
        juce::ValueTree icon;
        juce::Colour colour{ colours::blue500 };
        bool iconOnRight = false;
    };
} // namespace jive_demo::views
