#pragma once

namespace juce
{
    template <>
    struct VariantConverter<FlexBox::AlignContent>
    {
        static FlexBox::AlignContent fromVar(const var& v);
        static var toVar(FlexBox::AlignContent direction);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<FlexBox::AlignItems>
    {
        static FlexBox::AlignItems fromVar(const var& v);
        static var toVar(FlexBox::AlignItems direction);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<FlexBox::Direction>
    {
        static FlexBox::Direction fromVar(const var& v);
        static var toVar(FlexBox::Direction direction);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<FlexBox::JustifyContent>
    {
        static FlexBox::JustifyContent fromVar(const var& v);
        static var toVar(FlexBox::JustifyContent direction);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<FlexBox::Wrap>
    {
        static FlexBox::Wrap fromVar(const var& v);
        static var toVar(FlexBox::Wrap direction);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<FlexItem::AlignSelf>
    {
        static FlexItem::AlignSelf fromVar(const var& v);
        static var toVar(FlexItem::AlignSelf alignSelf);

    private:
        static const Array<var> options;
    };
} // namespace juce
