#pragma once

#include <BinaryData.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive_demo::typography
{
    static const auto rubik = juce::Typeface::createSystemTypefaceFor(binary_data::rubik_ttf,
                                                                      binary_data::rubik_ttfSize);
    static const auto rubikMonoOne = juce::Typeface::createSystemTypefaceFor(binary_data::rubikmonoone_ttf,
                                                                             binary_data::rubikmonoone_ttfSize);
    static const auto sourceCodePro = juce::Typeface::createSystemTypefaceFor(binary_data::sourcecodepro_ttf,
                                                                              binary_data::sourcecodepro_ttfSize);
} // namespace jive_demo::typography
