#pragma once

#include <BinaryData.h>
#include <jive_layouts/jive_layouts.h>

namespace jive_examples
{
    // Fonts can only be found by their family names while the returned
    // typefaces are alive.
    [[nodiscard]] inline std::array<juce::Typeface::Ptr, 3> registerFonts()
    {
        const auto registerFont = [](const char* data, int size) {
            return juce::Typeface::createSystemTypefaceFor(data, static_cast<std::size_t>(size));
        };

        return {
            registerFont(shared_resources::rubik_ttf, shared_resources::rubik_ttfSize),
            registerFont(shared_resources::rubikmonoone_ttf, shared_resources::rubikmonoone_ttfSize),
            registerFont(shared_resources::sourcecodepro_ttf, shared_resources::sourcecodepro_ttfSize),
        };
    }
} // namespace jive_examples
