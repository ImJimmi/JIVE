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

        template <typename... Files>
        SourceDirectories(Files&&... files)
            : directories{ std::forward<Files>(files)... }
        {
        }

        void add(const juce::File& file)
        {
            directories.addIfNotAlreadyThere(file);
        }

        void remove(const juce::File& file)
        {
            directories.removeFirstMatchingValue(file);
        }

        [[nodiscard]] SourceFile find(const juce::String& relativePath) const
        {
            for (const auto& directory : directories)
            {
                if (const SourceFile file{ directory.getChildFile(relativePath) }; file.exists())
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
        juce::Array<juce::File> directories;
    };
} // namespace jive
