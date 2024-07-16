#pragma once

#include <jive_core/geometry/jive_BorderRadii.h>
#include <jive_core/graphics/jive_Fill.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    template <typename T>
    struct Interpolate
    {
        [[nodiscard]] auto operator()(T start, T end, double proportion) const
        {
            if constexpr (std::is_integral<T>())
                return static_cast<T>(std::round(start + (end - start) * proportion));
            else
            {
                if constexpr (std::is_arithmetic<T>())
                    return static_cast<T>(start + (end - start) * proportion);
                else
                {
                    // You should write an implementation of jive::Interpolate for
                    // the current type!
                    jassertfalse;

                    if (juce::approximatelyEqual(proportion, 0.0))
                        return start;

                    return end;
                }
            }
        }
    };

    template <typename T>
    [[nodiscard]] T interpolate(T start, T end, double proportion)
    {
        return Interpolate<T>{}(start, end, proportion);
    }

    template <>
    struct Interpolate<juce::String>
    {
        [[nodiscard]] auto operator()(const juce::String& start,
                                      const juce::String& end,
                                      double proportion) const
        {
            const auto isNumeric = [](const auto& text) {
                return text.containsOnly("0123456789.-");
            };

            if (isNumeric(start) && isNumeric(end))
                return juce::String{ interpolate(start.getDoubleValue(), end.getDoubleValue(), proportion) };

            return end;
        }
    };

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

    template <typename T>
    struct Interpolate<juce::Array<T>>
    {
        [[nodiscard]] auto operator()(const juce::Array<T>& start,
                                      const juce::Array<T>& end,
                                      double proportion) const
        {
            juce::Array<T> result;

            for (auto i = 0; i < std::min(start.size(), end.size()); i++)
                result.add(interpolate(start.getUnchecked(i), end.getUnchecked(i), proportion));

            return result;
        }
    };

    template <>
    struct Interpolate<juce::Grid::Px>
    {
        [[nodiscard]] auto operator()(const juce::Grid::Px& start,
                                      const juce::Grid::Px& end,
                                      double proportion) const
        {
            return juce::Grid::Px{
                interpolate(start.pixels, end.pixels, proportion),
            };
        }
    };

    template <>
    struct Interpolate<juce::Grid::TrackInfo>
    {
        [[nodiscard]] juce::Grid::TrackInfo operator()(const juce::Grid::TrackInfo& start,
                                                       const juce::Grid::TrackInfo& end,
                                                       double proportion) const
        {
            if (start.isPixels() && end.isPixels())
            {
                return juce::Grid::Px{
                    interpolate(start.getSize(), end.getSize(), proportion),
                };
            }
            if (start.isFractional() && end.isFractional())
            {
                return juce::Grid::Fr{
                    juce::roundToInt(1000.f * interpolate(start.getSize(), end.getSize(), proportion)),
                };
            }

            return end;
        }
    };

    template <>
    struct Interpolate<juce::Colour>
    {
        [[nodiscard]] juce::Colour operator()(juce::Colour start, juce::Colour end, double proportion) const
        {
            return start.interpolatedWith(end, static_cast<float>(proportion));
        }
    };

    template <>
    struct Interpolate<Fill>
    {
        [[nodiscard]] Fill operator()(const Fill& start, const Fill& end, double proportion) const
        {
            if (start.getColour().has_value() && end.getColour().has_value())
            {
                return Fill{
                    interpolate(*start.getColour(), *end.getColour(), proportion),
                };
            }

            return Fill{};
        }
    };

    template <typename T>
    struct Interpolate<BorderRadii<T>>
    {
        [[nodiscard]] BorderRadii<T> operator()(BorderRadii<T> start, BorderRadii<T> end, double proportion) const
        {
            return BorderRadii<T>{
                interpolate(start.topLeft, end.topLeft, proportion),
                interpolate(start.topRight, end.topRight, proportion),
                interpolate(start.bottomRight, end.bottomRight, proportion),
                interpolate(start.bottomLeft, end.bottomLeft, proportion),
            };
        }
    };
} // namespace jive
