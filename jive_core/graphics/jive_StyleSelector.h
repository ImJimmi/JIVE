#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class StyleSelector
    {
    public:
        StyleSelector() = delete;
        StyleSelector(const juce::String& selector);
        StyleSelector& operator=(const juce::String& selector);
        StyleSelector(const StyleSelector& other) = default;
        StyleSelector(StyleSelector&& other) = default;
        StyleSelector& operator=(const StyleSelector& other) = default;
        StyleSelector& operator=(StyleSelector&& other) = default;
        ~StyleSelector() = default;

        [[nodiscard]] bool appliesTo(const juce::Component& component) const;
        [[nodiscard]] bool operator<(const StyleSelector& other) const;

    private:
        std::function<bool(const juce::Component&)> predicate;
        std::array<std::uint16_t, 3> precedence;
    };
} // namespace jive
