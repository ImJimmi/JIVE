#include "jive_Overflow.h"

namespace juce
{
    const std::unordered_map<jive::Overflow, String> VariantConverter<jive::Overflow>::stringsMap = {
        { jive::Overflow::hidden, "hidden" },
        { jive::Overflow::scroll, "scroll" },
    };
} // namespace juce
