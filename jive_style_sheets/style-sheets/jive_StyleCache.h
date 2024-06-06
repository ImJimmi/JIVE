#pragma once

#include <jive_core/jive_core.h>
#include <juce_graphics/juce_graphics.h>

namespace jive
{
    class StyleSheet;

    class StyleCache
    {
    public:
        struct CachedProperty
        {
            juce::var value;
            std::function<void()> update;
        };

        explicit StyleCache(const StyleSheet&);

        [[nodiscard]] juce::Font getFont() const;
        [[nodiscard]] Fill getBorderFill() const;
        [[nodiscard]] Fill getBackground() const;
        [[nodiscard]] Fill getForeground() const;
        [[nodiscard]] BorderRadii<float> getBorderRadii() const;

        void update();
        void update(const juce::Identifier& id);

    private:
        const StyleSheet& styleSheet;
        std::unordered_map<juce::String, CachedProperty> properties;
    };
} // namespace jive
