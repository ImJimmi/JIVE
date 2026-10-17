#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    // Files bundled into the binary by the jive_sources() CMake function. They
    // are found by the absolute paths they had when they were bundled.
    class EmbeddedSources
    {
    public:
        // Embeds a zip whose entries are stored by their paths relative to
        // the given directory.
        class Registration
        {
        public:
            Registration(const juce::File& rootDirectory, const void* zipData, std::size_t zipSize);
            ~Registration();

        private:
            JUCE_DECLARE_NON_COPYABLE(Registration)
        };

        // Whether the file is in a directory whose files were embedded. Those
        // files are never read from disk, even if the file wasn't embedded.
        [[nodiscard]] static bool isEmbedded(const juce::File& file);

        [[nodiscard]] static bool contains(const juce::File& file);
        [[nodiscard]] static std::optional<juce::MemoryBlock> load(const juce::File& file);
    };
} // namespace jive
