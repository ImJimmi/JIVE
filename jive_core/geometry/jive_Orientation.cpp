#include "jive_Orientation.h"

#include <jive_core/values/variant-converters/jive_VariantValidation.h>

namespace juce
{
    const Array<var> VariantConverter<jive::Orientation>::options = {
        "horizontal",
        "vertical",
    };

    jive::Orientation VariantConverter<jive::Orientation>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<jive::Orientation>(options.indexOf(v));
    }

    var VariantConverter<jive::Orientation>::toVar(const jive::Orientation& orientation)
    {
        const auto index = static_cast<int>(orientation);

        jassert(options.size() >= index);
        return options[index];
    }

    bool VariantConverter<jive::Orientation>::isValid(const var& v)
    {
        return v.toString().trim().equalsIgnoreCase("auto") || jive::isOneOf(v, options);
    }

    String VariantConverter<jive::Orientation>::getSyntax()
    {
        return jive::oneOfSyntax(options, { "auto" });
    }
} // namespace juce
