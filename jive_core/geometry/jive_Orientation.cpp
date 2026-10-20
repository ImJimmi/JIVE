#include "jive_Orientation.h"

namespace juce
{
    // NOLINTNEXTLINE(juce-prefer-std-containers)
    static const Array<var> orientationOptions{
        "horizontal",
        "vertical",
    };

    jive::Orientation VariantConverter<jive::Orientation>::fromVar(const var& v)
    {
        jassert(orientationOptions.contains(v));
        return static_cast<jive::Orientation>(orientationOptions.indexOf(v));
    }

    var VariantConverter<jive::Orientation>::toVar(const jive::Orientation& orientation)
    {
        const auto index = static_cast<int>(orientation);

        jassert(orientationOptions.size() >= index);
        return orientationOptions[index];
    }
} // namespace juce
