#pragma once

#include <BinaryData.h>
#include <jive_layouts/jive_layouts.h>

namespace jive_examples
{
    // Fonts can only be found by their family names while the returned
    // typefaces are alive.
    [[nodiscard]] inline std::array<juce::Typeface::Ptr, 6> registerFonts()
    {
        const auto registerFont = [](const char* data, int size) {
            return juce::Typeface::createSystemTypefaceFor(data, static_cast<std::size_t>(size));
        };

        return {
            registerFont(shared_resources::RubikRegular_ttf, shared_resources::RubikRegular_ttfSize),
            registerFont(shared_resources::RubikBold_ttf, shared_resources::RubikBold_ttfSize),
            registerFont(shared_resources::RubikItalic_ttf, shared_resources::RubikItalic_ttfSize),
            registerFont(shared_resources::RubikBoldItalic_ttf, shared_resources::RubikBoldItalic_ttfSize),
            registerFont(shared_resources::RubikMonoOneRegular_ttf, shared_resources::RubikMonoOneRegular_ttfSize),
            registerFont(shared_resources::SourceCodeProRegular_ttf, shared_resources::SourceCodeProRegular_ttfSize),
        };
    }
} // namespace jive_examples
