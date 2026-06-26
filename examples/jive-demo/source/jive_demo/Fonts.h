#pragma once

#include <BinaryData.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace jive_demo
{
    // Loading these custom typefaces registers them with JUCE's font system so
    // they can be referenced by name (e.g. "Rubik Mono One") from the style
    // sheet's "font-family" property. The statics must stay alive for the
    // lifetime of the app, so keep an instance of Fonts around.
    struct Fonts
    {
        const juce::Typeface::Ptr rubik = juce::Typeface::createSystemTypefaceFor(binary_data::rubik_ttf,
                                                                                  binary_data::rubik_ttfSize);
        const juce::Typeface::Ptr rubikMonoOne = juce::Typeface::createSystemTypefaceFor(binary_data::rubikmonoone_ttf,
                                                                                         binary_data::rubikmonoone_ttfSize);
        const juce::Typeface::Ptr sourceCodePro = juce::Typeface::createSystemTypefaceFor(binary_data::sourcecodepro_ttf,
                                                                                          binary_data::sourcecodepro_ttfSize);
    };
} // namespace jive_demo
