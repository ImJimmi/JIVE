#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    // Represents a specific state a component might be in, and includes
    // methods for check if this state applies to a given component, and
    // finding the most applicable properties according to this state.
    class InteractionState
    {
    public:
        enum Flag
        {
            focus = 1 << 0,
            hover = 1 << 1,
            active = 1 << 2,
            toggled = 1 << 3,
            disabled = 1 << 4,
        };

        // Constructs an InteractionState that represents the state of the
        // given component.
        explicit InteractionState(const juce::Component& component)
        {
            if (component.isMouseOverOrDragging(true))
                setHovered();
            if (component.isMouseButtonDown(true))
                setActive();
            if (!component.isEnabled())
                setDisabled();
            if (component.hasKeyboardFocus(false))
                setFocused();
            if (const auto* button = dynamic_cast<const juce::Button*>(&component);
                button != nullptr && button->getToggleState())
            {
                setToggled();
            }
        }

        // Constructs an InteractionState from one or more flags. This can
        // usually be called as an initializer-list for cleaner construction,
        // e.g.
        // ```
        // state = {InteractionState::hover, InteractionState::toggled}; // target components that are toggled-on, and are being hovered by the mouse.
        // state = InteractionState::focus; // Implicit constructor straight from a single flag.
        // ```
        template <typename... Flags>
        InteractionState(Flags... initialFlags)
            : flags{ initialFlags... }
        {
            static_assert((std::is_same_v<Flags, Flag> && ...));
        }

        InteractionState() = default;
        InteractionState(const InteractionState&) = default;
        InteractionState& operator=(const InteractionState&) = default;
        InteractionState(InteractionState&&) = default;
        InteractionState& operator=(InteractionState&&) = default;
        ~InteractionState() = default;

        InteractionState& setHovered()
        {
            flags.emplace(hover);
            return *this;
        }

        [[nodiscard]] auto isHovered() const
        {
            return flags.count(hover) > 0;
        }

        InteractionState& setActive()
        {
            flags.emplace(active);
            return *this;
        }

        [[nodiscard]] auto isActive() const
        {
            return flags.count(active) > 0;
        }

        InteractionState& setDisabled()
        {
            flags.emplace(disabled);
            return *this;
        }

        [[nodiscard]] auto isDisabled() const
        {
            return flags.count(disabled) > 0;
        }

        InteractionState& setFocused()
        {
            flags.emplace(focus);
            return *this;
        }

        [[nodiscard]] auto isFocused() const
        {
            return flags.count(focus) > 0;
        }

        InteractionState& setToggled()
        {
            flags.emplace(toggled);
            return *this;
        }

        [[nodiscard]] auto isToggled() const
        {
            return flags.count(toggled) > 0;
        }

        [[nodiscard]] auto asInt() const
        {
            auto value = 0;

            for (const auto& flag : flags)
                value += static_cast<int>(flag);

            return value;
        }

        // Returns true if this object represents the state of the given
        // component.
        [[nodiscard]] auto appliesTo(const juce::Component& component) const
        {
            auto toggleApplies = !flags.count(toggled);

            if (auto* button = dynamic_cast<const juce::Button*>(&component))
                toggleApplies = (flags.count(toggled) > 0 && button->getToggleState()) || flags.count(toggled) == 0;

            return !((flags.count(hover) > 0 && !component.isMouseOverOrDragging(true))
                     || (flags.count(active) > 0 && !component.isMouseButtonDown(true))
                     || (flags.count(disabled) > 0 && component.isEnabled())
                     || (flags.count(focus) > 0 && !component.hasKeyboardFocus(false))
                     || !toggleApplies);
        }

        // Searches the given set of properties for one with the given
        // baseProperty name that this object also represents.
        // Note that this does not only check for an exact match, but also
        // checks for other applicable states - e.g. the property
        // `"background.disabled"` would be considered applicable to an
        // InteractionState object with the flags
        // `{InteractionState::hover, InteractionState::disabled}` because the
        // property is _less_ specific than the current state.
        // `"background.hover.disabled"` would be _more_ applicable and so
        // would be the one returned by this method, but a property that's more
        // specific, e.g. `"background.hover.focus.disabled"` would not be
        // applicable.
        [[nodiscard]] std::optional<juce::var> findMostApplicable(const juce::NamedValueSet& properties,
                                                                  const juce::String& baseProperty) const
        {
            juce::Array<std::pair<juce::String, juce::StringArray>> keys;

            for (const auto& [key, value] : properties)
            {
                if (key.toString().startsWith(baseProperty))
                {
                    auto parts = juce::StringArray::fromTokens(key
                                                                   .toString()
                                                                   .fromFirstOccurrenceOf(baseProperty + ".", false, false),
                                                               ".",
                                                               "");
                    parts.removeDuplicates(true);
                    parts.sort(false);
                    keys.addIfNotAlreadyThere(std::make_pair(key.toString(), parts));
                }
            }

            static constexpr auto sum = static_cast<int>(Flag::focus)
                                      + static_cast<int>(Flag::disabled)
                                      + static_cast<int>(Flag::active)
                                      + static_cast<int>(Flag::hover)
                                      + static_cast<int>(Flag::toggled);

            for (auto bitset = sum; bitset >= 0; bitset--)
            {
                juce::StringArray parts;

                for (auto i = 0; i < 4; i++)
                {
                    if ((bitset & (1 << i)) != 0 && flags.count(static_cast<Flag>(1 << i)) > 0)
                        parts.add(names.at(static_cast<Flag>(1 << i)));
                }

                parts.sort(false);

                for (const auto& [property, propertyParts] : keys)
                {
                    if (parts == propertyParts)
                        return properties[property];
                }
            }

            return std::nullopt;
        }

    public:
        std::unordered_set<Flag> flags;
        static inline const std::unordered_map<Flag, juce::String> names{
            { Flag::hover, "hover" },
            { Flag::active, "active" },
            { Flag::disabled, "disabled" },
            { Flag::focus, "focus" },
            { Flag::toggled, "toggled" }
        };
    };
} // namespace jive
