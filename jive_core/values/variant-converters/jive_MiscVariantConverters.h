#pragma once

#include <jive_core/values/jive_Colours.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce
{
    template <>
    struct VariantConverter<Justification>
    {
        static Justification fromVar(const var& v)
        {
            if (v == "centred-left")
                return Justification::centredLeft;
            else if (v == "top-left")
                return Justification::topLeft;
            else if (v == "centred-top")
                return Justification::centredTop;
            else if (v == "top-right")
                return Justification::topRight;
            else if (v == "centred-right")
                return Justification::centredRight;
            else if (v == "bottom-right")
                return Justification::bottomRight;
            else if (v == "centred-bottom")
                return Justification::centredBottom;
            else if (v == "bottom-left")
                return Justification::bottomLeft;
            else if (v == "centred")
                return Justification::centred;

            return Justification::centred;
        }

        static var toVar(Justification justification)
        {
            if (justification == Justification::centredLeft)
                return "centred-left";
            else if (justification == Justification::topLeft)
                return "top-left";
            else if (justification == Justification::centredTop)
                return "centred-top";
            else if (justification == Justification::topRight)
                return "top-right";
            else if (justification == Justification::centredRight)
                return "centred-right";
            else if (justification == Justification::bottomRight)
                return "bottom-right";
            else if (justification == Justification::centredBottom)
                return "centred-bottom";
            else if (justification == Justification::bottomLeft)
                return "bottom-left";
            else if (justification == Justification::centred)
                return "centred";

            return {};
        }
    };

    template <typename ValueType>
    struct VariantConverter<BorderSize<ValueType>>
    {
        static BorderSize<ValueType> fromVar(const var& v)
        {
            const auto tokens = StringArray::fromTokens(v.toString(), false);

            if (tokens.size() == 1)
            {
                return BorderSize<ValueType>{ static_cast<float>(v) };
            }
            if (tokens.size() == 2)
            {
                const auto topAndBottom = static_cast<ValueType>(tokens[0].getDoubleValue());
                const auto leftAndRight = static_cast<ValueType>(tokens[1].getDoubleValue());

                return BorderSize<ValueType>{ topAndBottom, leftAndRight, topAndBottom, leftAndRight };
            }
            if (tokens.size() == 3)
            {
                const auto top = static_cast<ValueType>(tokens[0].getDoubleValue());
                const auto leftAndRight = static_cast<ValueType>(tokens[1].getDoubleValue());
                const auto bottom = static_cast<ValueType>(tokens[2].getDoubleValue());

                return BorderSize<ValueType>{ top, leftAndRight, bottom, leftAndRight };
            }
            if (tokens.size() == 4)
            {
                const auto top = static_cast<ValueType>(tokens[0].getDoubleValue());
                const auto right = static_cast<ValueType>(tokens[1].getDoubleValue());
                const auto bottom = static_cast<ValueType>(tokens[2].getDoubleValue());
                const auto left = static_cast<ValueType>(tokens[3].getDoubleValue());

                return BorderSize<ValueType>{ top, left, bottom, right };
            }

            return BorderSize<ValueType>{};
        }

        static var toVar(BorderSize<ValueType> border)
        {
            StringArray tokens;

            tokens.add(String{ static_cast<double>(border.getTop()) });
            tokens.add(String{ static_cast<double>(border.getRight()) });
            tokens.add(String{ static_cast<double>(border.getBottom()) });
            tokens.add(String{ static_cast<double>(border.getLeft()) });

            return tokens.joinIntoString(" ");
        }
    };

    template <>
    struct VariantConverter<Identifier>
    {
        static Identifier fromVar(const var& v)
        {
            if (v.toString().isEmpty())
                return {};

            return { v.toString() };
        }

        static var toVar(Identifier id)
        {
            return { id.toString() };
        }
    };

    template <>
    struct VariantConverter<MouseCursor::StandardCursorType>
    {
        static MouseCursor::StandardCursorType fromVar(const var& v)
        {
            jassert(options.contains(v));
            return static_cast<MouseCursor::StandardCursorType>(options.indexOf(v));
        }

        static var toVar(MouseCursor::StandardCursorType cursor)
        {
            const auto index = static_cast<int>(cursor);

            jassert(options.size() >= index);
            return options[index];
        }

    private:
        static const inline Array<var> options{
            "inherit",
            "none",
            "default",
            "wait",
            "text",
            "crosshair",
            "copy",
            "pointer",
            "grab",
            "left-right",
            "up-down",
            "move",
            "up",
            "down",
            "left",
            "right",
            "up-left",
            "up-right",
            "down-left",
            "down-right"
        };
    };

    template <typename T>
    struct VariantConverter<Array<T>>
    {
        static Array<T> fromVar(const var& v)
        {
            if (v.isArray())
            {
                auto& varArray = *v.getArray();
                Array<T> result;

                for (const auto& value : varArray)
                {
                    if (!value.isVoid())
                        result.add(VariantConverter<T>::fromVar(value));
                }

                return result;
            }

            if (v.isString())
            {
                const auto tokens = StringArray::fromTokens(v.toString(), " ", {});

                if (tokens.size() > 1)
                {
                    Array<T> result;

                    for (const auto& value : tokens)
                        result.add(VariantConverter<T>::fromVar(value));

                    return result;
                }
            }

            if (v.isVoid())
                return {};

            return { VariantConverter<T>::fromVar(v) };
        }

        static var toVar(const Array<T>& array)
        {
            var result;

            for (const auto& value : array)
                result.append(VariantConverter<T>::toVar(value));

            return result;
        }
    };

    template <>
    struct VariantConverter<StringArray>
    {
        static StringArray fromVar(const var& v)
        {
            if (v.isArray())
            {
                auto& varArray = *v.getArray();
                StringArray result;

                for (const auto& value : varArray)
                {
                    if (!value.isVoid())
                        result.add(value.toString());
                }

                return result;
            }

            return StringArray::fromTokens(v.toString(), " ,;|", "'\"");
        }

        static var toVar(const StringArray& array)
        {
            return array.joinIntoString(" ");
        }
    };

    template <>
    struct VariantConverter<Font>
    {
        static Font fromVar(const var& v)
        {
            return Font::fromString(v.toString());
        }

        static var toVar(const Font& font)
        {
            return font.toString();
        }
    };

    template <>
    struct VariantConverter<DocumentWindow::TitleBarButtons>
    {
        static DocumentWindow::TitleBarButtons fromVar(const var& v)
        {
            auto result = 0;

            const auto tokens = StringArray::fromTokens(v.toString(), false);

            if (tokens.contains("close"))
                result += DocumentWindow::closeButton;
            if (tokens.contains("minimise"))
                result += DocumentWindow::minimiseButton;
            if (tokens.contains("maximise"))
                result += DocumentWindow::maximiseButton;

            return static_cast<DocumentWindow::TitleBarButtons>(result);
        }

        static var toVar(const DocumentWindow::TitleBarButtons& flags)
        {
            StringArray tokens;

            if (flags & DocumentWindow::closeButton)
                tokens.add("close");
            if (flags & DocumentWindow::minimiseButton)
                tokens.add("minimise");
            if (flags & DocumentWindow::maximiseButton)
                tokens.add("maximise");

            return tokens.joinIntoString(" ");
        }
    };

    template <>
    class VariantConverter<Image>
    {
    public:
        static Image fromVar(const var& v)
        {
            return Image{
                dynamic_cast<ImagePixelData*>(v.getObject())
            };
        }

        static var toVar(const Image& image)
        {
            return var{ image.getPixelData() };
        }
    };

    template <>
    class VariantConverter<RectanglePlacement>
    {
    public:
        static RectanglePlacement fromVar(const var& v)
        {
            const auto tokens = StringArray::fromTokens(v.toString(), false);
            return std::accumulate(std::begin(tokens),
                                   std::end(tokens),
                                   0,
                                   [](const auto& flags, const auto& token) {
                                       jassert(map.count(token) > 0);
                                       return flags + map.at(token);
                                   });
        }

        static var toVar(const RectanglePlacement& placement)
        {
            auto stringFlagPair = std::find_if(std::begin(map),
                                               std::end(map),
                                               [&placement](const auto& pair) {
                                                   return placement.testFlags(pair.second);
                                               });
            jassert(stringFlagPair != std::end(map));

            return stringFlagPair->first;
        }

    private:
        static const inline std::unordered_map<String, RectanglePlacement::Flags> map{
            { "left", RectanglePlacement::xLeft },
            { "right", RectanglePlacement::xRight },
            { "centred-x", RectanglePlacement::xMid },
            { "top", RectanglePlacement::yTop },
            { "bottom", RectanglePlacement::yBottom },
            { "centred-y", RectanglePlacement::yMid },
            { "stretch", RectanglePlacement::stretchToFit },
            { "fill", RectanglePlacement::fillDestination },
            { "reduce-only", RectanglePlacement::onlyReduceInSize },
            { "increase-only", RectanglePlacement::onlyIncreaseInSize },
            { "do-not-resize", RectanglePlacement::doNotResize },
            { "centred", RectanglePlacement::centred },
        };
    };

    template <>
    class VariantConverter<Colour>
    {
    public:
        static Colour fromVar(const var& v)
        {
            if (v.isString())
                return jive::parseColour(v.toString());

            if (v.isInt() || v.isInt64())
                return Colour{ static_cast<uint32>(static_cast<int64>(v)) };

            return Colour{};
        }

        static var toVar(const Colour& colour)
        {
            return colour.toString();
        }
    };

    template <typename Arithmetic>
    class VariantConverter<Point<Arithmetic>>
    {
    public:
        static Point<Arithmetic> fromVar(const var& value)
        {
            const auto tokens = StringArray::fromTokens(value.toString(), ",", "");
            jassert(tokens.size() == 2);

            if constexpr (std::is_integral<Arithmetic>())
            {
                return {
                    static_cast<Arithmetic>(std::round(tokens[0].getDoubleValue())),
                    static_cast<Arithmetic>(std::round(tokens[1].getDoubleValue())),
                };
            }
            else
            {
                return {
                    static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                    static_cast<Arithmetic>(tokens[1].getDoubleValue()),
                };
            }
        }

        static var toVar(const Point<Arithmetic>& point)
        {
            return point.toString();
        }
    };

    template <typename Arithmetic>
    class VariantConverter<Rectangle<Arithmetic>>
    {
    public:
        static Rectangle<Arithmetic> fromVar(const var& value)
        {
            const auto tokens = StringArray::fromTokens(value.toString(), " ", "");

            return Rectangle{
                static_cast<Arithmetic>(tokens[0].getDoubleValue()),
                static_cast<Arithmetic>(tokens[1].getDoubleValue()),
                static_cast<Arithmetic>(tokens[2].getDoubleValue()),
                static_cast<Arithmetic>(tokens[3].getDoubleValue()),
            };
        }

        static var toVar(const Rectangle<Arithmetic>& rectangle)
        {
            return rectangle.toString();
        }
    };

    template <std::size_t N>
    class VariantConverter<char[N]>
    {
    public:
        static juce::String fromVar(const var& v)
        {
            return v.toString();
        }

        static var toVar(const char cString[N])
        {
            return var{ String{ cString } };
        }
    };

    template <>
    class VariantConverter<URL>
    {
    public:
        static URL fromVar(const var& v)
        {
            return { v.toString() };
        }

        static var toVar(const URL& url)
        {
            static constexpr auto includeGetParameters = true;
            return url.toString(includeGetParameters);
        }
    };
} // namespace juce
