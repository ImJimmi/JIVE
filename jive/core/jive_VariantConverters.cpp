#include "jive_VariantConverters.h"

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<jive::GuiItem::Display>::varArray = {
        "flex",
        "grid"
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
} // namespace juce
