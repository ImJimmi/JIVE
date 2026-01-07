#include "jive_FlexVariantConverters.h"

namespace juce
{
    const std::unordered_map<String, FlexBox::AlignContent> VariantConverter<FlexBox::AlignContent>::options{
        { "stretch", FlexBox::AlignContent::stretch },
        { "flex-start", FlexBox::AlignContent::flexStart },
        { "start", FlexBox::AlignContent::flexStart },
        { "flex-end", FlexBox::AlignContent::flexEnd },
        { "end", FlexBox::AlignContent::flexEnd },
        { "centre", FlexBox::AlignContent::center },
        { "center", FlexBox::AlignContent::center },
        { "space-between", FlexBox::AlignContent::spaceBetween },
        { "space-around", FlexBox::AlignContent::spaceAround },
    };

    FlexBox::AlignContent VariantConverter<FlexBox::AlignContent>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexBox::AlignContent::stretch;
    }

    var VariantConverter<FlexBox::AlignContent>::toVar(FlexBox::AlignContent alignContent)
    {
        for (const auto& [key, value] : options)
        {
            if (alignContent == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    const std::unordered_map<String, FlexBox::AlignItems> VariantConverter<FlexBox::AlignItems>::options{
        { "stretch", FlexBox::AlignItems::stretch },
        { "flex-start", FlexBox::AlignItems::flexStart },
        { "start", FlexBox::AlignItems::flexStart },
        { "flex-end", FlexBox::AlignItems::flexEnd },
        { "end", FlexBox::AlignItems::flexEnd },
        { "centre", FlexBox::AlignItems::center },
        { "center", FlexBox::AlignItems::center },
    };

    FlexBox::AlignItems VariantConverter<FlexBox::AlignItems>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexBox::AlignItems::stretch;
    }

    var VariantConverter<FlexBox::AlignItems>::toVar(FlexBox::AlignItems alignItems)
    {
        for (const auto& [key, value] : options)
        {
            if (alignItems == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    const std::unordered_map<String, FlexBox::Direction> VariantConverter<FlexBox::Direction>::options{
        { "row", FlexBox::Direction::row },
        { "row-reverse", FlexBox::Direction::rowReverse },
        { "column", FlexBox::Direction::column },
        { "column-reverse", FlexBox::Direction::columnReverse },
    };

    FlexBox::Direction VariantConverter<FlexBox::Direction>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexBox::Direction::column;
    }

    var VariantConverter<FlexBox::Direction>::toVar(FlexBox::Direction direction)
    {
        for (const auto& [key, value] : options)
        {
            if (direction == value)
                return key;
        }

        jassertfalse;
        return "column";
    }

    const std::unordered_map<String, FlexBox::JustifyContent> VariantConverter<FlexBox::JustifyContent>::options{
        { "flex-start", FlexBox::JustifyContent::flexStart },
        { "start", FlexBox::JustifyContent::flexStart },
        { "flex-end", FlexBox::JustifyContent::flexEnd },
        { "end", FlexBox::JustifyContent::flexEnd },
        { "centre", FlexBox::JustifyContent::center },
        { "center", FlexBox::JustifyContent::center },
        { "space-between", FlexBox::JustifyContent::spaceBetween },
        { "space-around", FlexBox::JustifyContent::spaceAround },
    };

    FlexBox::JustifyContent VariantConverter<FlexBox::JustifyContent>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexBox::JustifyContent::flexStart;
    }

    var VariantConverter<FlexBox::JustifyContent>::toVar(FlexBox::JustifyContent justifyContent)
    {
        for (const auto& [key, value] : options)
        {
            if (justifyContent == value)
                return key;
        }

        jassertfalse;
        return "flex-start";
    }

    const std::unordered_map<String, FlexBox::Wrap> VariantConverter<FlexBox::Wrap>::options{
        { "nowrap", FlexBox::Wrap::noWrap },
        { "no-wrap", FlexBox::Wrap::noWrap },
        { "wrap", FlexBox::Wrap::wrap },
        { "wrap-reverse", FlexBox::Wrap::wrapReverse },
    };

    FlexBox::Wrap VariantConverter<FlexBox::Wrap>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexBox::Wrap::noWrap;
    }

    var VariantConverter<FlexBox::Wrap>::toVar(FlexBox::Wrap wrap)
    {
        for (const auto& [key, value] : options)
        {
            if (wrap == value)
                return key;
        }

        jassertfalse;
        return "nowrap";
    }

    const std::unordered_map<String, FlexItem::AlignSelf> VariantConverter<FlexItem::AlignSelf>::options{
        { "auto", FlexItem::AlignSelf::autoAlign },
        { "flex-start", FlexItem::AlignSelf::flexStart },
        { "start", FlexItem::AlignSelf::flexStart },
        { "flex-end", FlexItem::AlignSelf::flexEnd },
        { "end", FlexItem::AlignSelf::flexEnd },
        { "centre", FlexItem::AlignSelf::center },
        { "center", FlexItem::AlignSelf::center },
        { "stretch", FlexItem::AlignSelf::stretch },
    };

    FlexItem::AlignSelf VariantConverter<FlexItem::AlignSelf>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return FlexItem::AlignSelf::autoAlign;
    }

    var VariantConverter<FlexItem::AlignSelf>::toVar(FlexItem::AlignSelf alignSelf)
    {
        for (const auto& [key, value] : options)
        {
            if (alignSelf == value)
                return key;
        }

        jassertfalse;
        return "auto";
    }
} // namespace juce
