#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace jive
{
    // A view, style sheet or other file that JIVE reads. A relative path is
    // found in the roots created by the jive_sources() CMake function, which
    // may be embedded in the binary. An absolute path is read from disk.
    class SourceFile
    {
    public:
        SourceFile() = default;
        explicit SourceFile(const juce::String& relativeOrAbsolutePath);
        explicit SourceFile(const juce::File& file);

        [[nodiscard]] const juce::String& getPath() const;
        [[nodiscard]] SourceFile getChildFile(const juce::String& relativePath) const;
        [[nodiscard]] SourceFile getParentDirectory() const;

        // Returns juce::File{} if the file is embedded, or can't be found.
        [[nodiscard]] juce::File getFileOnDisk() const;

        [[nodiscard]] bool isEmbedded() const;
        [[nodiscard]] bool exists() const;
        [[nodiscard]] juce::String loadAsString() const;
        [[nodiscard]] std::vector<std::byte> loadAsData() const;

        [[nodiscard]] bool operator==(const SourceFile& other) const;
        [[nodiscard]] bool operator!=(const SourceFile& other) const;

    private:
        [[nodiscard]] bool isAbsolute() const;

        juce::String path;
    };

    // Returns the file the tree, or its closest ancestor, was parsed from.
    [[nodiscard]] SourceFile findSourceFile(const juce::ValueTree& tree);
} // namespace jive

namespace std
{
    template <>
    struct hash<jive::SourceFile>
    {
        [[nodiscard]] std::size_t operator()(const jive::SourceFile& file) const noexcept
        {
            return std::hash<juce::String>{}(file.getPath());
        }
    };
} // namespace std
