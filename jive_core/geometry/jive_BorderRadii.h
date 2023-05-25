#pragma once

namespace jive
{
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
            return topLeft == other.topLeft
                && topRight == other.topRight
                && bottomRight == other.bottomRight
                && bottomLeft == other.bottomLeft;
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
} // namespace juce
