#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    // Zips the given contents by their paths, for testing embedded sources.
    [[nodiscard]] inline juce::MemoryBlock createZip(const std::vector<std::pair<juce::String, juce::String>>& files)
    {
        juce::ZipFile::Builder builder;

        for (const auto& [path, contents] : files)
        {
            builder.addEntry(new juce::MemoryInputStream{ contents.toRawUTF8(), contents.getNumBytesAsUTF8(), true },
                             9,
                             path,
                             juce::Time::getCurrentTime());
        }

        juce::MemoryBlock zip;

        {
            juce::MemoryOutputStream stream{ zip, false };
            builder.writeToStream(stream, nullptr);
        }

        return zip;
    }
} // namespace jive
