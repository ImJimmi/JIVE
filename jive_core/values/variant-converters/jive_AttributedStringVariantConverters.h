#pragma once

#include <juce_graphics/juce_graphics.h>

namespace juce
{
    template <>
    class VariantConverter<AttributedString::ReadingDirection>
    {
    public:
        static AttributedString::ReadingDirection fromVar(const var& v);
        static var toVar(const AttributedString::ReadingDirection& direction);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const Array<var> options;
    };

    template <>
    class VariantConverter<AttributedString::WordWrap>
    {
    public:
        static AttributedString::WordWrap fromVar(const var& v);
        static var toVar(const AttributedString::WordWrap& wordWrap);
        static bool isValid(const var& v);
        static String getSyntax();

    private:
        static const Array<var> options;
    };
} // namespace juce
