#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace juce
{
    template <>
    struct VariantConverter<GridItem::JustifySelf>
    {
        static GridItem::JustifySelf fromVar(const var& v);
        static var toVar(GridItem::JustifySelf justification);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, GridItem::JustifySelf> options;
    };

    template <>
    struct VariantConverter<GridItem::AlignSelf>
    {
        static GridItem::AlignSelf fromVar(const var& v);
        static var toVar(GridItem::AlignSelf alignment);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, GridItem::AlignSelf> options;
    };

    template <>
    struct VariantConverter<GridItem::Span>
    {
        static GridItem::Span fromVar(const var& v);
        static var toVar(GridItem::Span span);
        static bool isValid(const var& v);
    };

    template <>
    struct VariantConverter<GridItem::Property>
    {
        static GridItem::Property fromVar(const var& v);
        static var toVar(GridItem::Property startAndEnd);
        static bool isValid(const var& v);
    };

    template <>
    struct VariantConverter<GridItem::StartAndEndProperty>
    {
        static GridItem::StartAndEndProperty fromVar(const var& v);
        static var toVar(GridItem::StartAndEndProperty startAndEnd);
        static bool isValid(const var& v);
    };

    template <>
    struct VariantConverter<Grid::JustifyItems>
    {
        static Grid::JustifyItems fromVar(const var& v);
        static var toVar(Grid::JustifyItems justification);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, Grid::JustifyItems> options;
    };

    template <>
    struct VariantConverter<Grid::AlignItems>
    {
        static Grid::AlignItems fromVar(const var& v);
        static var toVar(Grid::AlignItems alignment);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, Grid::AlignItems> options;
    };

    template <>
    struct VariantConverter<Grid::JustifyContent>
    {
        static Grid::JustifyContent fromVar(const var& v);
        static var toVar(Grid::JustifyContent alignment);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, Grid::JustifyContent> options;
    };

    template <>
    struct VariantConverter<Grid::AlignContent>
    {
        static Grid::AlignContent fromVar(const var& v);
        static var toVar(Grid::AlignContent alignment);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, Grid::AlignContent> options;
    };

    template <>
    struct VariantConverter<Grid::AutoFlow>
    {
        static Grid::AutoFlow fromVar(const var& v);
        static var toVar(Grid::AutoFlow flow);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const std::unordered_map<String, Grid::AutoFlow> options;
    };

    template <>
    struct VariantConverter<Grid::TrackInfo>
    {
        static Grid::TrackInfo fromVar(const var& v);
        static var toVar(const Grid::TrackInfo& info);
        static bool isValid(const var& v);
    };

    template <>
    struct VariantConverter<Grid::Px>
    {
        static Grid::Px fromVar(const var& v);
        static var toVar(const Grid::Px& px);
        static bool isValid(const var& v);
    };
} // namespace juce
