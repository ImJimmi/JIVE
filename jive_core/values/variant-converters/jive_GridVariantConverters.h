#pragma once

namespace juce
{
    template <>
    struct VariantConverter<GridItem::JustifySelf>
    {
        static GridItem::JustifySelf fromVar(const var& v);
        static var toVar(GridItem::JustifySelf justification);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<GridItem::AlignSelf>
    {
        static GridItem::AlignSelf fromVar(const var& v);
        static var toVar(GridItem::AlignSelf alignment);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<GridItem::Span>
    {
        static GridItem::Span fromVar(const var& v);
        static var toVar(GridItem::Span span);
    };

    template <>
    struct VariantConverter<GridItem::Property>
    {
        static GridItem::Property fromVar(const var& v);
        static var toVar(GridItem::Property startAndEnd);
    };

    template <>
    struct VariantConverter<GridItem::StartAndEndProperty>
    {
        static GridItem::StartAndEndProperty fromVar(const var& v);
        static var toVar(GridItem::StartAndEndProperty startAndEnd);
    };

    template <>
    struct VariantConverter<Grid::JustifyItems>
    {
        static Grid::JustifyItems fromVar(const var& v);
        static var toVar(Grid::JustifyItems justification);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<Grid::AlignItems>
    {
        static Grid::AlignItems fromVar(const var& v);
        static var toVar(Grid::AlignItems alignment);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<Grid::JustifyContent>
    {
        static Grid::JustifyContent fromVar(const var& v);
        static var toVar(Grid::JustifyContent alignment);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<Grid::AlignContent>
    {
        static Grid::AlignContent fromVar(const var& v);
        static var toVar(Grid::AlignContent alignment);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<Grid::AutoFlow>
    {
        static Grid::AutoFlow fromVar(const var& v);
        static var toVar(Grid::AutoFlow flow);

    private:
        static const Array<var> options;
    };

    template <>
    struct VariantConverter<Grid::TrackInfo>
    {
        static Grid::TrackInfo fromVar(const var& v);
        static var toVar(const Grid::TrackInfo& info);
    };

    template <>
    struct VariantConverter<Grid::Px>
    {
        static Grid::Px fromVar(const var& v);
        static var toVar(const Grid::Px& px);
    };
} // namespace juce
