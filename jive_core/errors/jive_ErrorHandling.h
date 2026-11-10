#pragma once

#include <jive_core/files/jive_SourceFile.h>
#include <jive_core/values/jive_Object.h>

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    void logValueError(juce::ValueTree tree,
                       juce::Identifier id,
                       juce::StringRef message = "");

    void logValueError(Object::ReferenceCountedPointer object,
                       juce::Identifier id,
                       juce::StringRef message = "",
                       const SourceFile& sourceFile = {});
} // namespace jive
