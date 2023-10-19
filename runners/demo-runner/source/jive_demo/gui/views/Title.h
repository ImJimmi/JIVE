#pragma once

#include <jive_core/jive_core.h>

namespace jive_demo::views
{
    class Title
    {
    public:
        [[nodiscard]] auto& withText(const juce::String& title) noexcept
        {
            text = title;
            return *this;
        }

        [[nodiscard]] auto& withColour(const juce::Colour& textColour) noexcept
        {
            colour = textColour;
            return *this;
        }

        [[nodiscard]] auto& withMargin(const juce::String& marginString) noexcept
        {
            margin = marginString;
            return *this;
        }

        [[nodiscard]] auto operator()() const
        {
            const auto style = [this] {
                auto object = new jive::Object{
                    { "font-family", "Rubik Mono One" },
                    { "font-size", 35 },
                };

                if (colour.has_value())
                    object->setProperty("foreground", jive::toVar(colour));

                return object;
            };

            return juce::ValueTree{
                "Text",
                {
                    { "text", text },
                    { "margin", margin },
                    { "style", style() },
                },
            };
        }

    private:
        juce::String text;
        std::optional<juce::Colour> colour;
        juce::String margin = "0 0 10 0";
    };
} // namespace jive_demo::views
