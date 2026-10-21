#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    // Somewhere relative SourceFile paths are found: a directory on disk, or a
    // zip embedded in the binary. The jive_sources() CMake function creates
    // one for each call. Where that's not available, such as in a Projucer
    // project, create them as static objects instead. Roots are searched in
    // the order they were created, for as long as they exist.
    class SourceRoot
    {
    public:
        explicit SourceRoot(const juce::File& rootDirectory);
        SourceRoot(const void* zipDataToUse, std::size_t zipSizeInBytes);
        ~SourceRoot();

        // Returns the first root with a file at the given relative path, or
        // nullptr if none have one.
        [[nodiscard]] static const SourceRoot* find(const juce::String& relativePath);

        [[nodiscard]] bool isEmbedded() const;
        [[nodiscard]] bool contains(const juce::String& relativePath) const;
        [[nodiscard]] juce::File getFileOnDisk(const juce::String& relativePath) const;
        [[nodiscard]] juce::MemoryBlock load(const juce::String& relativePath) const;

    private:
        const juce::File directory;
        const void* const zipData = nullptr;
        const std::size_t zipSize = 0;

        JUCE_DECLARE_NON_COPYABLE(SourceRoot)
    };
} // namespace jive
