#pragma once

#include <jive_core/jive_core.h>

namespace jive
{
    struct StyleIdentifier
    {
        [[nodiscard]] auto operator==(const StyleIdentifier& other) const noexcept
        {
            return id == other.id
                && className == other.className
                && type == other.type
                && enabled == other.enabled
                && keyboard == other.keyboard
                && mouse == other.mouse
                && toggled == other.toggled;
        }

        juce::String id;
        juce::String className;
        juce::String type;
        bool enabled = true;
        ComponentInteractionState::Keyboard keyboard;
        ComponentInteractionState::Mouse mouse;
        bool toggled = false;
    };
} // namespace jive

namespace std
{
    template <>
    struct hash<jive::StyleIdentifier>
    {
        [[nodiscard]] auto operator()(const jive::StyleIdentifier& styleID) const noexcept
        {
            return (styleID.id
                    + styleID.className
                    + styleID.type
                    + juce::String{ styleID.enabled ? "enabled" : "disabled" }
                    + juce::String{ static_cast<int>(styleID.keyboard) }
                    + juce::String{ static_cast<int>(styleID.mouse) }
                    + juce::String{ styleID.toggled ? "checked" : "unchecked" })
                .hash();
        }
    };
} // namespace std
