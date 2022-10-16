#pragma once

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    template <>
    struct VariantConverter<Justification>
    {
        //==============================================================================================================
        static Justification fromVar(const var& v);
        static var toVar(Justification justification);
    };

    //==================================================================================================================
    template <typename ValueType>
    struct VariantConverter<BorderSize<ValueType>>
    {
        //==============================================================================================================
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

    //==================================================================================================================
    template <>
    struct VariantConverter<Identifier>
    {
        //==============================================================================================================
        static Identifier fromVar(const var& v);
        static var toVar(Identifier id);
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<MouseCursor::StandardCursorType>
    {
        //==============================================================================================================
        static MouseCursor::StandardCursorType fromVar(const var& v);
        static var toVar(MouseCursor::StandardCursorType cursor);

    private:
        //==============================================================================================================
        static const Array<var> options;
    };

    //==================================================================================================================
    template <typename T>
    struct VariantConverter<Array<T>>
    {
        //==============================================================================================================
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

    //==================================================================================================================
    template <>
    struct VariantConverter<StringArray>
    {
        //==============================================================================================================
        static StringArray fromVar(const var& v);
        static var toVar(const StringArray& info);
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<Font>
    {
        //==============================================================================================================
        static Font fromVar(const var& v);
        static var toVar(const Font& font);
    };

    //==================================================================================================================
    template <>
    struct VariantConverter<DocumentWindow::TitleBarButtons>
    {
        //==============================================================================================================
        static DocumentWindow::TitleBarButtons fromVar(const var& v);
        static var toVar(const DocumentWindow::TitleBarButtons& flags);
    };

    //==================================================================================================================
    template <>
    class VariantConverter<Image>
    {
    public:
        //==============================================================================================================
        static Image fromVar(const var& v);
        static var toVar(const Image& image);
    };

    //==================================================================================================================
    template <>
    class VariantConverter<RectanglePlacement>
    {
    public:
        //==============================================================================================================
        static RectanglePlacement fromVar(const var& v);
        static var toVar(const RectanglePlacement& placement);

    private:
        //==============================================================================================================
        static const std::unordered_map<String, RectanglePlacement::Flags> map;
    };

    //==================================================================================================================
    template <>
    class VariantConverter<Colour>
    {
    public:
        //==============================================================================================================
        static Colour fromVar(const var& v);
        static var toVar(const Colour& colour);
    };

    //==================================================================================================================
    template <typename Arithmetic>
    class VariantConverter<Point<Arithmetic>>
    {
    public:
        //==============================================================================================================
        static Point<Arithmetic> fromVar(const var& v);
        static var toVar(const Point<Arithmetic>& point);
    };
} // namespace juce
