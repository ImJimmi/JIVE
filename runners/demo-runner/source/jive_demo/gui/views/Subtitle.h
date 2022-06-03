#pragma once

#include <jive_core/jive_core.h>

namespace jive_demo::views
{
    class Subtitle
    {
    public:
        [[nodiscard]] auto& withText(const juce::String& subtitle) noexcept
        {
            text = subtitle;
            return *this;
        }

        [[nodiscard]] auto operator()() const
        {
            return juce::ValueTree{
                "Text",
                {
                    { "text", text },
                    { "style", style() },
                },
            };
        }

    private:
        static constexpr auto style = [] {
            return new jive::Object{
                { "font-size", 20 },
            };
        };

        juce::String text;
    };
} // namespace jive_demo::views
