#include "jive_AttributedStringVariantConverters.h"
#include "jive_VariantValidation.h"

namespace juce
{
    const Array<var> VariantConverter<AttributedString::ReadingDirection>::options = {
        "ltr",
        "rtl",
    };

    AttributedString::ReadingDirection VariantConverter<AttributedString::ReadingDirection>::fromVar(const var& v)
    {
        jassert(options.contains(v));

        if (v == "ltr")
            return AttributedString::ReadingDirection::leftToRight;
        if (v == "rtl")
            return AttributedString::ReadingDirection::rightToLeft;

        return AttributedString::ReadingDirection::natural;
    }

    var VariantConverter<AttributedString::ReadingDirection>::toVar(const AttributedString::ReadingDirection& direction)
    {
        switch (direction)
        {
        case AttributedString::ReadingDirection::leftToRight:
            return "ltr";
        case AttributedString::ReadingDirection::rightToLeft:
            return "rtl";
        case AttributedString::ReadingDirection::natural:
            break;
        }

        return {};
    }

    const Array<var> VariantConverter<AttributedString::WordWrap>::options = {
        "none",
        "by-word",
        "by-character",
    };

    AttributedString::WordWrap VariantConverter<AttributedString::WordWrap>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<AttributedString::WordWrap>(options.indexOf(v));
    }

    var VariantConverter<AttributedString::WordWrap>::toVar(const AttributedString::WordWrap& wordWrap)
    {
        const auto index = static_cast<int>(wordWrap);

        jassert(options.size() >= index);
        return options[index];
    }

    bool VariantConverter<AttributedString::ReadingDirection>::isValid(const var& v)
    {
        return jive::isOneOf(v, options);
    }

    String VariantConverter<AttributedString::ReadingDirection>::getSyntax()
    {
        return jive::oneOfSyntax(options);
    }

    bool VariantConverter<AttributedString::WordWrap>::isValid(const var& v)
    {
        return jive::isOneOf(v, options);
    }

    String VariantConverter<AttributedString::WordWrap>::getSyntax()
    {
        return jive::oneOfSyntax(options);
    }
} // namespace juce
