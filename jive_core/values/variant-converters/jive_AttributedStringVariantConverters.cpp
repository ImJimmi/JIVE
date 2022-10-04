#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<AttributedString::ReadingDirection>::options = {
        "natural",
        "left-to-right",
        "right-to-left",
    };

    AttributedString::ReadingDirection VariantConverter<AttributedString::ReadingDirection>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<AttributedString::ReadingDirection>(options.indexOf(v));
    }

    var VariantConverter<AttributedString::ReadingDirection>::toVar(const AttributedString::ReadingDirection& direction)
    {
        const auto index = static_cast<int>(direction);

        jassert(options.size() >= index);
        return options[index];
    }

    //==================================================================================================================
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
} // namespace juce
