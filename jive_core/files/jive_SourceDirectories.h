#pragma once

#include "jive_SourceFile.h"

namespace jive
{
    class SourceDirectories : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<SourceDirectories>;

        SourceDirectories() = default;
        SourceDirectories(const SourceDirectories&) = default;
        SourceDirectories(SourceDirectories&&) = default;
        SourceDirectories& operator=(const SourceDirectories&) = default;
        SourceDirectories& operator=(SourceDirectories&&) = default;
        ~SourceDirectories() = default;

        template <typename... Directories>
        SourceDirectories(Directories&&... directoriesToSearch)
            : directories{ SourceFile{ std::forward<Directories>(directoriesToSearch) }... }
        {
        }

        void add(const SourceFile& directory)
        {
            directories.addIfNotAlreadyThere(directory);
        }

        void add(const juce::File& directory)
        {
            add(SourceFile{ directory });
        }

        void remove(const SourceFile& directory)
        {
            directories.removeFirstMatchingValue(directory);
        }

        void remove(const juce::File& directory)
        {
            remove(SourceFile{ directory });
        }

        // Paths starting with "./" or "../" are relative to the file that
        // refers to them. Other paths are looked for in each directory.
        [[nodiscard]] SourceFile find(const juce::String& relativePath, const SourceFile& referrer = {}) const
        {
            if (relativePath.startsWith("./") || relativePath.startsWith("../"))
            {
                const auto file = referrer.getParentDirectory().getChildFile(relativePath);
                return file.exists() ? file : SourceFile{};
            }

            for (const auto& directory : directories)
            {
                if (const auto file = directory.getChildFile(relativePath); file.exists())
                    return file;
            }

            return {};
        }

        [[nodiscard]] auto size() const
        {
            return static_cast<std::size_t>(directories.size());
        }

        [[nodiscard]] auto begin() const
        {
            return directories.begin();
        }

        [[nodiscard]] auto end() const
        {
            return directories.end();
        }

    private:
        juce::Array<SourceFile> directories;
    };
} // namespace jive
