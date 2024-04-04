#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    [[nodiscard]] juce::String buildProgressBar(double progressNormalised, int maxStringLength = 50);
} // namespace jive
