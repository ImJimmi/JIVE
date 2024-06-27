#pragma once

namespace jive
{
    enum class LayoutStrategy
    {
        real,
        dummy,
    };

    [[nodiscard]] inline juce::String toString(LayoutStrategy strategy)
    {
        switch (strategy)
        {
        case LayoutStrategy::real:
            return "real";
        case LayoutStrategy::dummy:
            return "dummy";
        }

        jassertfalse;
        return "";
    }
} // namespace jive
