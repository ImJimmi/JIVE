#include <jive_layouts/jive_layouts.h>

namespace juce
{
    const Array<var> VariantConverter<FlexBox::AlignContent>::options = {
        "stretch",
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    FlexBox::AlignContent VariantConverter<FlexBox::AlignContent>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexBox{}.alignContent;

        jassert(options.contains(v));
        return static_cast<FlexBox::AlignContent>(options.indexOf(v));
    }

    var VariantConverter<FlexBox::AlignContent>::toVar(FlexBox::AlignContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(options.size() >= index);
        return options[index];
    }

    const Array<var> VariantConverter<FlexBox::AlignItems>::options = {
        "stretch",
        "flex-start",
        "flex-end",
        "centre"
    };

    FlexBox::AlignItems VariantConverter<FlexBox::AlignItems>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexBox{}.alignItems;

        jassert(options.contains(v));
        return static_cast<FlexBox::AlignItems>(options.indexOf(v));
    }

    var VariantConverter<FlexBox::AlignItems>::toVar(FlexBox::AlignItems alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(options.size() >= index);
        return options[index];
    }

    const Array<var> VariantConverter<FlexBox::Direction>::options = {
        "row",
        "row-reverse",
        "column",
        "column-reverse"
    };

    FlexBox::Direction VariantConverter<FlexBox::Direction>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexBox{}.flexDirection;

        jassert(options.contains(v));
        return static_cast<FlexBox::Direction>(options.indexOf(v));
    }

    var VariantConverter<FlexBox::Direction>::toVar(FlexBox::Direction direction)
    {
        const auto index = static_cast<int>(direction);

        jassert(options.size() >= index);
        return options[index];
    }

    const Array<var> VariantConverter<FlexBox::JustifyContent>::options = {
        "flex-start",
        "flex-end",
        "centre",
        "space-between",
        "space-around"
    };

    FlexBox::JustifyContent VariantConverter<FlexBox::JustifyContent>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexBox{}.justifyContent;

        jassert(options.contains(v));
        return static_cast<FlexBox::JustifyContent>(options.indexOf(v));
    }

    var VariantConverter<FlexBox::JustifyContent>::toVar(FlexBox::JustifyContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(options.size() >= index);
        return options[index];
    }

    const Array<var> VariantConverter<FlexBox::Wrap>::options = {
        "nowrap",
        "wrap",
        "wrap-reverse"
    };

    FlexBox::Wrap VariantConverter<FlexBox::Wrap>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexBox{}.flexWrap;

        jassert(options.contains(v));
        return static_cast<FlexBox::Wrap>(options.indexOf(v));
    }

    var VariantConverter<FlexBox::Wrap>::toVar(FlexBox::Wrap wrap)
    {
        const auto index = static_cast<int>(wrap);

        jassert(options.size() >= index);
        return options[index];
    }

    const Array<var> VariantConverter<FlexItem::AlignSelf>::options = {
        "auto",
        "flex-start",
        "flex-end",
        "centre",
        "stretch"
    };

    FlexItem::AlignSelf VariantConverter<FlexItem::AlignSelf>::fromVar(const var& v)
    {
        if (v.isVoid())
            return FlexItem{}.alignSelf;

        jassert(options.contains(v));
        return static_cast<FlexItem::AlignSelf>(options.indexOf(v));
    }

    var VariantConverter<FlexItem::AlignSelf>::toVar(FlexItem::AlignSelf alignSelf)
    {
        const auto index = static_cast<int>(alignSelf);

        jassert(options.size() >= index);
        return options[index];
    }
} // namespace juce
