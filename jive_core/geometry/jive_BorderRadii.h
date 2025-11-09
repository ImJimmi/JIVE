#pragma once

#include <jive_core/algorithms/jive_Interpolate.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    struct Styles;

    template <typename Arithmetic>
    struct BorderRadii
    {
        static_assert(std::is_arithmetic<Arithmetic>());

        BorderRadii() = default;

        BorderRadii(Arithmetic initialValue)
            : BorderRadii{
                initialValue,
                initialValue,
            }
        {
        }

        BorderRadii(Arithmetic topLeftAndBottomRight,
                    Arithmetic bottomLeftAndTopRight)
            : BorderRadii{
                topLeftAndBottomRight,
                bottomLeftAndTopRight,
                topLeftAndBottomRight,
            }
        {
        }

        BorderRadii(Arithmetic initialTopLeft,
                    Arithmetic bottomLeftAndTopRight,
                    Arithmetic initialBottomRight)
            : BorderRadii{
                initialTopLeft,
                bottomLeftAndTopRight,
                initialBottomRight,
                bottomLeftAndTopRight,
            }
        {
        }

        BorderRadii(Arithmetic initialTopLeft,
                    Arithmetic initialTopRight,
                    Arithmetic initialBottomRight,
                    Arithmetic initialBottomLeft)
            : topLeft{ initialTopLeft }
            , topRight{ initialTopRight }
            , bottomRight{ initialBottomRight }
            , bottomLeft{ initialBottomLeft }
        {
        }

        BorderRadii(const BorderRadii&) = default;
        BorderRadii& operator=(const BorderRadii&) = default;
        BorderRadii(BorderRadii&&) = default;
        BorderRadii& operator=(BorderRadii&&) = default;
        ~BorderRadii() = default;

        bool operator==(const BorderRadii<Arithmetic>& other) const
        {
            return juce::approximatelyEqual(topLeft, other.topLeft)
                && juce::approximatelyEqual(topRight, other.topRight)
                && juce::approximatelyEqual(bottomRight, other.bottomRight)
                && juce::approximatelyEqual(bottomLeft, other.bottomLeft);
        }

        bool operator!=(const BorderRadii<Arithmetic>& other) const
        {
            return !(*this == other);
        }

        Arithmetic topLeft{ 0 };
        Arithmetic topRight{ 0 };
        Arithmetic bottomRight{ 0 };
        Arithmetic bottomLeft{ 0 };
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

    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Component& component,
                                                     const Styles& styles,
                                                     BorderRadii<float> defaultRadius = BorderRadii{ 0.0f });
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Button& component,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::PopupMenu& popup,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::PopupMenu::Item& item,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::ComboBox& comboBox,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::GroupComponent& group,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::Slider& slider,
                                                     const Styles& styles);
    [[nodiscard]] BorderRadii<float> getBorderRadius(const juce::ProgressBar& bar,
                                                     const Styles& styles);
} // namespace jive

namespace juce
{
    template <typename Arithmetic>
    struct VariantConverter<jive::BorderRadii<Arithmetic>>
    {
        static var toVar(const jive::BorderRadii<Arithmetic>& radii)
        {
            return juce::StringArray{
                juce::String{ radii.topLeft },
                juce::String{ radii.topRight },
                juce::String{ radii.bottomRight },
                juce::String{ radii.bottomLeft },
            }
                .joinIntoString(" ");
        }

        static jive::BorderRadii<Arithmetic> fromVar(const var& value)
        {
            const auto tokens = juce::StringArray::fromTokens(value.toString(),
                                                              " ",
                                                              "");

            if (tokens.size() == 1)
                return {
                    static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                };
            if (tokens.size() == 2)
                return {
                    static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[1].getDoubleValue()),
                };
            if (tokens.size() == 3)
                return {
                    static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[1].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[2].getDoubleValue()),
                };
            if (tokens.size() >= 4)
                return {
                    static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[1].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[2].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[3].getDoubleValue()),
                };

            return {};
        }
    };

    template <typename T>
    String& operator<<(String& str, const jive::BorderRadii<T>& border)
    {
        return str << "jive::BorderRadii<"
                   << typeid(T).name()
                   << "> { "
                   << border.topLeft << ", "
                   << border.topRight << ", "
                   << border.bottomRight << ", "
                   << border.bottomLeft
                   << " }";
    }
} // namespace juce
