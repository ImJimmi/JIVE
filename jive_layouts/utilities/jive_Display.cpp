#include "jive_Display.h"

#include <jive_core/values/variant-converters/jive_VariantValidation.h>

namespace juce
{
    const Array<var> VariantConverter<jive::Display>::options = {
        "flex",
        "grid",
        "block"
    };

    jive::Display VariantConverter<jive::Display>::fromVar(const var& v)
    {
        if (v.isVoid())
            return jive::Display::flex;

        jassert(options.contains(v));
        return static_cast<jive::Display>(options.indexOf(v));
    }

    var VariantConverter<jive::Display>::toVar(jive::Display display)
    {
        const auto index = static_cast<int>(display);

        jassert(options.size() >= index);
        return options[index];
    }

    bool VariantConverter<jive::Display>::isValid(const var& v)
    {
        return jive::isOneOf(v, options);
    }

    String VariantConverter<jive::Display>::getSyntax()
    {
        return jive::oneOfSyntax(options);
    }
} // namespace juce
