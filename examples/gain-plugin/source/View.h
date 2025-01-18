#include <jive_core/jive_core.h>

namespace jive_example
{
    [[nodiscard]] inline auto GainKnob()
    {
        return juce::ValueTree{
            "Knob",
            {
                { "id", "gain-knob" },
                { "width", 80 },
                { "height", 80 },
            },
        };
    }

    [[nodiscard]] inline auto GainLabel()
    {
        return juce::ValueTree{
            "Text",
            {
                { "id", "gain-label" },
                {
                    "style",
                    new jive::Object{
                        { "foreground", "#4BB1D9" },
                        { "font-size", 20 },
                        { "letter-spacing", 1.5 },
                    },
                },
            },
            {
                juce::ValueTree{
                    "Text",
                    {
                        { "text", "dB" },
                        {
                            "style",
                            new jive::Object{
                                { "font-weight", "bold" },
                            },
                        },
                    },
                },
            },
        };
    }

    [[nodiscard]] inline auto getGainPluginView()
    {
        return juce::ValueTree{
            "Editor",
            {
                { "width", 400 },
                { "height", 250 },
                { "align-items", "centre" },
                { "justify-content", "centre" },
                {
                    "style",
                    new jive::Object{
                        { "background", "#323D42" },
                    },
                },
            },
            {
                GainKnob(),
                GainLabel(),
            }
        };
    }
} // namespace jive_example
