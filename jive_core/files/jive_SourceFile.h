#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    // A view, style sheet or other file that JIVE reads. Files that were
    // embedded with the jive_sources() CMake function are read from the binary,
    // and everything else from disk.
    class SourceFile
    {
    public:
        SourceFile() = default;
        explicit SourceFile(const juce::File& file);

        [[nodiscard]] const juce::File& getFile() const;
        [[nodiscard]] bool isEmbedded() const;
        [[nodiscard]] bool exists() const;
        [[nodiscard]] juce::String loadAsString() const;
        [[nodiscard]] juce::MemoryBlock loadAsData() const;

    private:
        juce::File file;
    };
} // namespace jive
