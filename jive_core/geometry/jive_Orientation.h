#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    enum class Orientation
    {
        horizontal,
        vertical,
    };
} // namespace jive

namespace juce
{
    template <>
    class VariantConverter<jive::Orientation>
    {
    public:
        static jive::Orientation fromVar(const var& v);
        static var toVar(const jive::Orientation& orientation);

    private:
        static const Array<var> options;
    };
} // namespace juce
