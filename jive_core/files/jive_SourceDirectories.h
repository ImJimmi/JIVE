#pragma once

#include <juce_core/juce_core.h>

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

        [[nodiscard]] auto find(const juce::String& relativePath) const
        {
            for (const auto& file : directories)
            {
                if (auto child = file.getChildFile(relativePath); child.existsAsFile())
                    return child;
            }

            return juce::File{};
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
