#include "jive_Display.h"

//======================================================================================================================
namespace juce
{
    //==================================================================================================================
    const Array<var> VariantConverter<jive::Display>::options = {
        "flex",
        "grid",
        "block"
    };

    jive::Display VariantConverter<jive::Display>::fromVar(const var& v)
    {
        jassert(options.contains(v));
        return static_cast<jive::Display>(options.indexOf(v));
    }

    var VariantConverter<jive::Display>::toVar(jive::Display display)
    {
        const auto index = static_cast<int>(display);

        jassert(options.size() >= index);
        return options[index];
    }
} // namespace juce
