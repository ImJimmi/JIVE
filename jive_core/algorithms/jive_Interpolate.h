#pragma once

namespace jive
{
    template <typename T>
    struct Interpolate
    {
        [[nodiscard]] auto operator()(T start, T end, double proportion) const
        {
            if constexpr (std::is_integral<T>())
                return static_cast<T>(std::round(start + (end - start) * proportion));
            else if constexpr (std::is_arithmetic<T>())
                return static_cast<T>(start + (end - start) * proportion);
            else
            {
                if (juce::approximatelyEqual(proportion, 0.0))
                    return start;

                return end;
            }
        }
    };

    template <typename T>
    [[nodiscard]] T interpolate(T start, T end, double proportion)
    {
        return Interpolate<T>{}(start, end, proportion);
    }

    template <typename T>
    struct Interpolate<juce::BorderSize<T>>
    {
        [[nodiscard]] auto operator()(juce::BorderSize<T> start,
                                      juce::BorderSize<T> end,
                                      double proportion) const
        {
            return juce::BorderSize<T>{
                interpolate(start.getTop(), end.getTop(), proportion),
                interpolate(start.getLeft(), end.getLeft(), proportion),
                interpolate(start.getBottom(), end.getBottom(), proportion),
                interpolate(start.getRight(), end.getRight(), proportion),
            };
        }
    };
} // namespace jive
