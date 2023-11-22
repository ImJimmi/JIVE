#pragma once

namespace jive
{
    [[nodiscard]] juce::String buildProgressBar(double progressNormalised, int maxStringLength = 50);
} // namespace jive
