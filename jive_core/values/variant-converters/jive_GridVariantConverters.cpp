#include "jive_GridVariantConverters.h"

namespace juce
{
    const std::unordered_map<String, GridItem::JustifySelf> VariantConverter<GridItem::JustifySelf>::options = {
        { "start", GridItem::JustifySelf::start },
        { "end", GridItem::JustifySelf::end },
        { "centre", GridItem::JustifySelf::center },
        { "center", GridItem::JustifySelf::center },
        { "stretch", GridItem::JustifySelf::stretch },
        { "auto", GridItem::JustifySelf::autoValue },
    };

    GridItem::JustifySelf VariantConverter<GridItem::JustifySelf>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return GridItem::JustifySelf::stretch;
    }

    var VariantConverter<GridItem::JustifySelf>::toVar(GridItem::JustifySelf justifySelf)
    {
        for (const auto& [key, value] : options)
        {
            if (justifySelf == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    const std::unordered_map<String, GridItem::AlignSelf> VariantConverter<GridItem::AlignSelf>::options = {
        { "start", GridItem::AlignSelf::start },
        { "end", GridItem::AlignSelf::end },
        { "centre", GridItem::AlignSelf::center },
        { "center", GridItem::AlignSelf::center },
        { "stretch", GridItem::AlignSelf::stretch },
        { "auto", GridItem::AlignSelf::autoValue },
    };

    GridItem::AlignSelf VariantConverter<GridItem::AlignSelf>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return GridItem::AlignSelf::stretch;
    }

    var VariantConverter<GridItem::AlignSelf>::toVar(GridItem::AlignSelf alignSelf)
    {
        for (const auto& [key, value] : options)
        {
            if (alignSelf == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    GridItem::Span VariantConverter<GridItem::Span>::fromVar(const var& v)
    {
        const auto tokens = StringArray::fromTokens(v.toString(), false);
        jassert(tokens.size() == 2);
        jassert(tokens[0] == "span");

        return GridItem::Span{ tokens[1].getIntValue() };
    }

    var VariantConverter<GridItem::Span>::toVar(GridItem::Span span)
    {
        return "span " + String{ span.number };
    }

    GridItem::Property VariantConverter<GridItem::Property>::fromVar(const var& v)
    {
        if (v.isInt())
            return static_cast<int>(v);

        const auto value = v.toString();

        if (value == "auto")
            return GridItem::Keyword::autoValue;

        if (value.startsWith("span"))
            return VariantConverter<GridItem::Span>::fromVar(v);

        return value;
    }

    var VariantConverter<GridItem::Property>::toVar(GridItem::Property property)
    {
        if (property.hasSpan())
        {
            return VariantConverter<GridItem::Span>::toVar(GridItem::Span{ property.getNumber(),
                                                                           property.getName() });
        }

        if (property.hasAbsolute())
            return property.getNumber();
        if (property.hasName())
            return property.getName();

        return "auto";
    }

    GridItem::StartAndEndProperty VariantConverter<GridItem::StartAndEndProperty>::fromVar(const var& v)
    {
        auto tokens = StringArray::fromTokens(v.toString(), "/", "");
        jassert(tokens.size() == 2);

        Array<var> vars;

        for (auto& token : tokens)
        {
            token = token.trim();

            if (token.containsOnly("0123456789"))
                vars.add(token.getIntValue());
            else
                vars.add(token);
        }

        const auto start = VariantConverter<GridItem::Property>::fromVar(vars[0]);
        const auto end = VariantConverter<GridItem::Property>::fromVar(vars[1]);

        return { start, end };
    }

    var VariantConverter<GridItem::StartAndEndProperty>::toVar(GridItem::StartAndEndProperty startAndEnd)
    {
        StringArray tokens;

        tokens.add(VariantConverter<GridItem::Property>::toVar(startAndEnd.start));
        tokens.add(VariantConverter<GridItem::Property>::toVar(startAndEnd.end));

        return tokens.joinIntoString(" / ");
    }

    const std::unordered_map<String, Grid::JustifyItems> VariantConverter<Grid::JustifyItems>::options = {
        { "start", Grid::JustifyItems::start },
        { "end", Grid::JustifyItems::end },
        { "centre", Grid::JustifyItems::center },
        { "center", Grid::JustifyItems::center },
        { "stretch", Grid::JustifyItems::stretch },
    };

    Grid::JustifyItems VariantConverter<Grid::JustifyItems>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return Grid::JustifyItems::stretch;
    }

    var VariantConverter<Grid::JustifyItems>::toVar(Grid::JustifyItems justifyItems)
    {
        for (const auto& [key, value] : options)
        {
            if (justifyItems == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    const std::unordered_map<String, Grid::AlignItems> VariantConverter<Grid::AlignItems>::options = {
        { "start", Grid::AlignItems::start },
        { "end", Grid::AlignItems::end },
        { "centre", Grid::AlignItems::center },
        { "center", Grid::AlignItems::center },
        { "stretch", Grid::AlignItems::stretch },
    };

    Grid::AlignItems VariantConverter<Grid::AlignItems>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return Grid::AlignItems::stretch;
    }

    var VariantConverter<Grid::AlignItems>::toVar(Grid::AlignItems alignItems)
    {
        for (const auto& [key, value] : options)
        {
            if (alignItems == value)
                return key;
        }

        jassertfalse;
        return "stretch";
    }

    const std::unordered_map<String, Grid::JustifyContent> VariantConverter<Grid::JustifyContent>::options = {
        { "start", Grid::JustifyContent::start },
        { "end", Grid::JustifyContent::end },
        { "centre", Grid::JustifyContent::center },
        { "center", Grid::JustifyContent::center },
        { "stretch", Grid::JustifyContent::stretch },
        { "space-around", Grid::JustifyContent::spaceAround },
        { "space-between", Grid::JustifyContent::spaceBetween },
        { "space-evenly", Grid::JustifyContent::spaceEvenly },
    };

    Grid::JustifyContent VariantConverter<Grid::JustifyContent>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return Grid::JustifyContent::start;
    }

    var VariantConverter<Grid::JustifyContent>::toVar(Grid::JustifyContent justifyContent)
    {
        for (const auto& [key, value] : options)
        {
            if (justifyContent == value)
                return key;
        }

        jassertfalse;
        return "start";
    }

    const std::unordered_map<String, Grid::AlignContent> VariantConverter<Grid::AlignContent>::options = {
        { "start", Grid::AlignContent::start },
        { "end", Grid::AlignContent::end },
        { "centre", Grid::AlignContent::center },
        { "center", Grid::AlignContent::center },
        { "stretch", Grid::AlignContent::stretch },
        { "space-around", Grid::AlignContent::spaceAround },
        { "space-between", Grid::AlignContent::spaceBetween },
        { "space-evenly", Grid::AlignContent::spaceEvenly },
    };

    Grid::AlignContent VariantConverter<Grid::AlignContent>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return Grid::AlignContent::start;
    }

    var VariantConverter<Grid::AlignContent>::toVar(Grid::AlignContent alignContent)
    {
        for (const auto& [key, value] : options)
        {
            if (alignContent == value)
                return key;
        }

        jassertfalse;
        return "start";
    }

    const std::unordered_map<String, Grid::AutoFlow> VariantConverter<Grid::AutoFlow>::options = {
        { "row", Grid::AutoFlow::row },
        { "column", Grid::AutoFlow::column },
        { "row dense", Grid::AutoFlow::rowDense },
        { "row-dense", Grid::AutoFlow::rowDense },
        { "column dense", Grid::AutoFlow::columnDense },
        { "column-dense", Grid::AutoFlow::columnDense },
    };

    Grid::AutoFlow VariantConverter<Grid::AutoFlow>::fromVar(const var& v)
    {
        if (options.count(v.toString()) >= 1)
            return options.at(v.toString());

        return Grid::AutoFlow::row;
    }

    var VariantConverter<Grid::AutoFlow>::toVar(Grid::AutoFlow autoFlow)
    {
        for (const auto& [key, value] : options)
        {
            if (autoFlow == value)
                return key;
        }

        jassertfalse;
        return "start";
    }

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
} // namespace juce
