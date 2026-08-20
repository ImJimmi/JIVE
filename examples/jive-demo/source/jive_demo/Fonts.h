#pragma once

#include <BinaryData.h>
#include <juce_graphics/juce_graphics.h>

namespace jive_demo::fonts
{
    const auto rubik = juce::Typeface::createSystemTypefaceFor(binary_data::rubik_ttf,
                                                               binary_data::rubik_ttfSize);
    const auto rubikMonoOne = juce::Typeface::createSystemTypefaceFor(binary_data::rubikmonoone_ttf,
                                                                      binary_data::rubikmonoone_ttfSize);
    const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(binary_data::sourcecodepro_ttf,
                                                                       binary_data::sourcecodepro_ttfSize);
} // namespace jive_demo::fonts
