#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<GridItem::JustifySelf>::options = {
        "start",
        "end",
        "centre",
        "stretch",
        "auto"
    };

    GridItem::JustifySelf VariantConverter<GridItem::JustifySelf>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<GridItem::JustifySelf>(options.indexOf(v));
    }

    var VariantConverter<GridItem::JustifySelf>::toVar(GridItem::JustifySelf justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<GridItem::AlignSelf>::options = {
        "start",
        "end",
        "centre",
        "stretch",
        "auto"
    };

    GridItem::AlignSelf VariantConverter<GridItem::AlignSelf>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<GridItem::AlignSelf>(options.indexOf(v));
    }

    var VariantConverter<GridItem::AlignSelf>::toVar(GridItem::AlignSelf alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
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

    //==================================================================================================================
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

    //==================================================================================================================
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

    //==================================================================================================================
    const Array<var> VariantConverter<Grid::JustifyItems>::options = {
        "start",
        "end",
        "centre",
        "stretch",
    };

    Grid::JustifyItems VariantConverter<Grid::JustifyItems>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<Grid::JustifyItems>(options.indexOf(v));
    }

    var VariantConverter<Grid::JustifyItems>::toVar(Grid::JustifyItems justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<Grid::AlignItems>::options = {
        "start",
        "end",
        "centre",
        "stretch",
    };

    Grid::AlignItems VariantConverter<Grid::AlignItems>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<Grid::AlignItems>(options.indexOf(v));
    }

    var VariantConverter<Grid::AlignItems>::toVar(Grid::AlignItems alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<Grid::JustifyContent>::options = {
        "start",
        "end",
        "centre",
        "stretch",
        "space-around",
        "space-between",
        "space-evenly",
    };

    Grid::JustifyContent VariantConverter<Grid::JustifyContent>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<Grid::JustifyContent>(options.indexOf(v));
    }

    var VariantConverter<Grid::JustifyContent>::toVar(Grid::JustifyContent justification)
    {
        const auto index = static_cast<int>(justification);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<Grid::AlignContent>::options = {
        "start",
        "end",
        "centre",
        "stretch",
        "space-around",
        "space-between",
        "space-evenly",
    };

    Grid::AlignContent VariantConverter<Grid::AlignContent>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<Grid::AlignContent>(options.indexOf(v));
    }

    var VariantConverter<Grid::AlignContent>::toVar(Grid::AlignContent alignment)
    {
        const auto index = static_cast<int>(alignment);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
    const Array<var> VariantConverter<Grid::AutoFlow>::options = {
        "row",
        "column",
        "row dense",
        "column dense",
    };

    Grid::AutoFlow VariantConverter<Grid::AutoFlow>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<Grid::AutoFlow>(options.indexOf(v));
    }

    var VariantConverter<Grid::AutoFlow>::toVar(Grid::AutoFlow flow)
    {
        const auto index = static_cast<int>(flow);

        jassert(options.size() >= index);
        return options[index];
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
