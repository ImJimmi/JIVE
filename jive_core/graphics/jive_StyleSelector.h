#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class StyleSelector
    {
    public:
        StyleSelector() = delete;
        StyleSelector(const juce::String& selector, juce::Component* root = nullptr);
        StyleSelector(const char* selector, juce::Component* root = nullptr);
        StyleSelector(const StyleSelector& other) = default;
        StyleSelector(StyleSelector&& other) = default;
        StyleSelector& operator=(const StyleSelector& other) = default;
        StyleSelector& operator=(StyleSelector&& other) = default;
        ~StyleSelector() = default;

        [[nodiscard]] bool appliesTo(const juce::Component& component) const;
        [[nodiscard]] std::array<std::uint16_t, 3> getPrecedence() const;

        // Returns true if this selector has a higher precedence than (should be
        // chosen over) the other
        [[nodiscard]] bool operator<(const StyleSelector& other) const;

        // Returns true if this selector has a lower precedence than the other
        [[nodiscard]] bool operator>(const StyleSelector& other) const;

        // Returns true if this selector has equal precedence to the other
        [[nodiscard]] bool operator==(const StyleSelector& other) const;

        // A special type of selector used to apply inline styles to the given
        // component that always has precedence over any other selectors that may
        // be applicable to the component
        [[nodiscard]] static StyleSelector makeInlineStyleSelector(const juce::Component&);

        // A special type of selector used to apply system styles (e.g. from
        // themes, default/fallback styles, etc. defined by JIVE internally) to
        // the given component that never have precedence over any user-defined
        // selectors
        [[nodiscard]] static StyleSelector makeSystemStyleSelector(const juce::String&);

    private:
        std::function<bool(const juce::Component&)> predicate;
        std::array<std::uint16_t, 3> precedence;
        bool isSystem = false;
    };
} // namespace jive
