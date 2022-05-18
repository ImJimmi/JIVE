#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<jive::GuiItem::Display>::varArray = {
        "flex",
        "grid",
        "block"
    };

    jive::GuiItem::Display VariantConverter<jive::GuiItem::Display>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<jive::GuiItem::Display>(varArray.indexOf(v));
    }

    var VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display display)
    {
        const auto index = static_cast<int>(display);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::Direction>::varArray = {
        "row",
        "row-reverse",
        "column",
        "column-reverse"
    };

    juce::FlexBox::Direction VariantConverter<juce::FlexBox::Direction>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::Direction>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction direction)
    {
        const auto index = static_cast<int>(direction);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::Wrap>::varArray = {
        "nowrap",
        "wrap",
        "wrap-reverse"
    };

    juce::FlexBox::Wrap VariantConverter<juce::FlexBox::Wrap>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::Wrap>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap wrap)
    {
        const auto index = static_cast<int>(wrap);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::JustifyContent>::varArray = {
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    juce::FlexBox::JustifyContent VariantConverter<juce::FlexBox::JustifyContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::JustifyContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::AlignItems>::varArray = {
        "stretch",
        "flex-start",
        "flex-end",
        "centre"
    };

    juce::FlexBox::AlignItems VariantConverter<juce::FlexBox::AlignItems>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::AlignItems>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexBox::AlignContent>::varArray = {
        "stretch",
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    juce::FlexBox::AlignContent VariantConverter<juce::FlexBox::AlignContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexBox::AlignContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::FlexItem::AlignSelf>::varArray = {
        "auto",
        "flex-start",
        "flex-end",
        "centre",
        "stretch"
    };

    juce::FlexItem::AlignSelf VariantConverter<juce::FlexItem::AlignSelf>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::FlexItem::AlignSelf>(varArray.indexOf(v));
    }

    var VariantConverter<juce::FlexItem::AlignSelf>::toVar(juce::FlexItem::AlignSelf alignSelf)
    {
        const auto index = static_cast<int>(alignSelf);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    Justification VariantConverter<Justification>::fromVar(const var& v)
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

    var VariantConverter<Justification>::toVar(Justification justification)
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

    //==================================================================================================================
    template <typename ValueType>
    BorderSize<ValueType> VariantConverter<BorderSize<ValueType>>::fromVar(const var& v)
    {
        const auto tokens = juce::StringArray::fromTokens(v.toString(), false);

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

    template <typename ValueType>
    var VariantConverter<BorderSize<ValueType>>::toVar(BorderSize<ValueType> border)
    {
        juce::StringArray tokens;

        tokens.add(String{ static_cast<double>(border.getTop()) });
        tokens.add(String{ static_cast<double>(border.getRight()) });
        tokens.add(String{ static_cast<double>(border.getBottom()) });
        tokens.add(String{ static_cast<double>(border.getLeft()) });

        return tokens.joinIntoString(" ");
    }

    //==================================================================================================================
    juce::Identifier VariantConverter<juce::Identifier>::fromVar(const var& v)
    {
        if (v.toString().isEmpty())
            return {};

        return { v.toString() };
    }

    var VariantConverter<juce::Identifier>::toVar(juce::Identifier id)
    {
        return { id.toString() };
    }

    //==================================================================================================================
    const Array<var> VariantConverter<MouseCursor::StandardCursorType>::options = {
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

    MouseCursor::StandardCursorType VariantConverter<MouseCursor::StandardCursorType>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<MouseCursor::StandardCursorType>(options.indexOf(v));
    }

    var VariantConverter<MouseCursor::StandardCursorType>::toVar(MouseCursor::StandardCursorType cursor)
    {
        const auto index = static_cast<int>(cursor);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::GridItem::JustifySelf>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
        "auto"
    };

    juce::GridItem::JustifySelf VariantConverter<juce::GridItem::JustifySelf>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::GridItem::JustifySelf>(varArray.indexOf(v));
    }

    var VariantConverter<juce::GridItem::JustifySelf>::toVar(juce::GridItem::JustifySelf justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::GridItem::AlignSelf>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
        "auto"
    };

    juce::GridItem::AlignSelf VariantConverter<juce::GridItem::AlignSelf>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::GridItem::AlignSelf>(varArray.indexOf(v));
    }

    var VariantConverter<juce::GridItem::AlignSelf>::toVar(juce::GridItem::AlignSelf alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    juce::GridItem::Span VariantConverter<juce::GridItem::Span>::fromVar(const var& v)
    {
        const auto tokens = juce::StringArray::fromTokens(v.toString(), false);
        jassert(tokens.size() == 2);
        jassert(tokens[0] == "span");

        return juce::GridItem::Span{ tokens[1].getIntValue() };
    }

    var VariantConverter<juce::GridItem::Span>::toVar(juce::GridItem::Span span)
    {
        return "span " + String{ span.number };
    }

    //==================================================================================================================
    juce::GridItem::Property VariantConverter<juce::GridItem::Property>::fromVar(const var& v)
    {
        if (v.isInt())
            return static_cast<int>(v);

        const auto value = v.toString();

        if (value == "auto")
            return juce::GridItem::Keyword::autoValue;

        if (value.startsWith("span"))
            return VariantConverter<juce::GridItem::Span>::fromVar(v);

        return value;
    }

    var VariantConverter<juce::GridItem::Property>::toVar(juce::GridItem::Property property)
    {
        if (property.hasSpan())
        {
            return VariantConverter<juce::GridItem::Span>::toVar(juce::GridItem::Span{ property.getNumber(),
                                                                                       property.getName() });
        }

        if (property.hasAbsolute())
            return property.getNumber();
        if (property.hasName())
            return property.getName();

        return "auto";
    }

    //==================================================================================================================
    juce::GridItem::StartAndEndProperty VariantConverter<juce::GridItem::StartAndEndProperty>::fromVar(const var& v)
    {
        auto tokens = juce::StringArray::fromTokens(v.toString(), "/", "");
        jassert(tokens.size() == 2);

        juce::Array<juce::var> vars;

        for (auto& token : tokens)
        {
            token = token.trim();

            if (token.containsOnly("0123456789"))
                vars.add(token.getIntValue());
            else
                vars.add(token);
        }

        const auto start = juce::VariantConverter<juce::GridItem::Property>::fromVar(vars[0]);
        const auto end = juce::VariantConverter<juce::GridItem::Property>::fromVar(vars[1]);

        return { start, end };
    }

    var VariantConverter<juce::GridItem::StartAndEndProperty>::toVar(juce::GridItem::StartAndEndProperty startAndEnd)
    {
        juce::StringArray tokens;

        tokens.add(juce::VariantConverter<juce::GridItem::Property>::toVar(startAndEnd.start));
        tokens.add(juce::VariantConverter<juce::GridItem::Property>::toVar(startAndEnd.end));

        return tokens.joinIntoString(" / ");
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::Grid::JustifyItems>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
    };

    juce::Grid::JustifyItems VariantConverter<juce::Grid::JustifyItems>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::Grid::JustifyItems>(varArray.indexOf(v));
    }

    var VariantConverter<juce::Grid::JustifyItems>::toVar(juce::Grid::JustifyItems justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::Grid::AlignItems>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
    };

    juce::Grid::AlignItems VariantConverter<juce::Grid::AlignItems>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::Grid::AlignItems>(varArray.indexOf(v));
    }

    var VariantConverter<juce::Grid::AlignItems>::toVar(juce::Grid::AlignItems alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::Grid::JustifyContent>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
        "space-around",
        "space-between",
        "space-evenly",
    };

    juce::Grid::JustifyContent VariantConverter<juce::Grid::JustifyContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::Grid::JustifyContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::Grid::JustifyContent>::toVar(juce::Grid::JustifyContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::Grid::AlignContent>::varArray = {
        "start",
        "end",
        "centre",
        "stretch",
        "space-around",
        "space-between",
        "space-evenly",
    };

    juce::Grid::AlignContent VariantConverter<juce::Grid::AlignContent>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::Grid::AlignContent>(varArray.indexOf(v));
    }

    var VariantConverter<juce::Grid::AlignContent>::toVar(juce::Grid::AlignContent alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<juce::Grid::AutoFlow>::varArray = {
        "row",
        "column",
        "row dense",
        "column dense",
    };

    juce::Grid::AutoFlow VariantConverter<juce::Grid::AutoFlow>::fromVar(const var& v)
    {
        jassert(varArray.contains(v));
        return static_cast<juce::Grid::AutoFlow>(varArray.indexOf(v));
    }

    var VariantConverter<juce::Grid::AutoFlow>::toVar(juce::Grid::AutoFlow flow)
    {
        const auto index = static_cast<int>(flow);

        jassert(varArray.size() >= index);
        return varArray[index];
    }

    //==================================================================================================================
    template <typename T>
    Array<T> VariantConverter<Array<T>>::fromVar(const var& v)
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
            const auto tokens = juce::StringArray::fromTokens(v.toString(), " ", {});

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

    template <typename T>
    var VariantConverter<Array<T>>::toVar(const Array<T>& array)
    {
        var result;

        for (const auto& value : array)
            result.append(VariantConverter<T>::toVar(value));

        return result;
    }

    //==================================================================================================================
    Grid::TrackInfo VariantConverter<Grid::TrackInfo>::fromVar(const var& v)
    {
        if (v.isString())
        {
            const auto text = v.toString();

            if (text == "auto")
                return {};

            const auto doubleValue = text.getDoubleValue();

            if (text.endsWithIgnoreCase("fr"))
                return Grid::Fr{ roundToInt(doubleValue) };

            return Grid::Px{ static_cast<long double>(doubleValue) };
        }

        if (v.isDouble())
            return Grid::Px{ static_cast<long double>(static_cast<double>(v)) };

        return Grid::Px{ static_cast<int>(v) };
    }

    var VariantConverter<Grid::TrackInfo>::toVar(const Grid::TrackInfo& info)
    {
        if (info.isAuto())
            return "auto";

        if (info.isFractional())
            return String{ roundToInt(info.getSize()) } + "fr";

        return info.getSize();
    }

    //==================================================================================================================
    StringArray VariantConverter<StringArray>::fromVar(const var& v)
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

        return juce::StringArray::fromTokens(v.toString(), " ", {});
    }

    var VariantConverter<StringArray>::toVar(const StringArray& array)
    {
        return array.joinIntoString(" ");
    }

    //==================================================================================================================
    Grid::Px VariantConverter<Grid::Px>::fromVar(const var& v)
    {
        if (v.isString())
            return Grid::Px{ static_cast<long double>(v.toString().getDoubleValue()) };

        if (v.isDouble())
            return Grid::Px{ static_cast<long double>(static_cast<double>(v)) };

        return Grid::Px{ static_cast<int>(v) };
    }

    var VariantConverter<Grid::Px>::toVar(const Grid::Px& px)
    {
        return String{ static_cast<double>(px.pixels) } + "px";
    }

    //==================================================================================================================
    jive::Width VariantConverter<jive::Width>::fromVar(const var& v)
    {
        jive::Width width;
        const auto text = v.toString();

        if (text.endsWith("%"))
        {
            width.setPercent(text.getFloatValue());
            return width;
        }

        width.setPixels(static_cast<float>(v));
        return width;
    }

    var VariantConverter<jive::Width>::toVar(const jive::Width& width)
    {
        if (width.isPixels())
            return width.getPixels();

        return juce::String{ width.getPercent() } + "%";
    }

    //==================================================================================================================
    jive::Height VariantConverter<jive::Height>::fromVar(const var& v)
    {
        jive::Height height;
        const auto text = v.toString();

        if (text.endsWith("%"))
        {
            height.setPercent(text.getFloatValue());
            return height;
        }

        height.setPixels(static_cast<float>(v));
        return height;
    }

    var VariantConverter<jive::Height>::toVar(const jive::Height& height)
    {
        if (height.isPixels())
            return height.getPixels();

        return juce::String{ height.getPercent() } + "%";
    }

    //==================================================================================================================
    juce::Font VariantConverter<juce::Font>::fromVar(const var& v)
    {
        return juce::Font::fromString(v.toString());
    }

    var VariantConverter<juce::Font>::toVar(const juce::Font& font)
    {
        return font.toString();
    }
} // namespace juce
