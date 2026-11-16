#include "jive_SourceRoot.h"

#include <jive_core/compile-time-helpers/jive_JuceVersion.h>

namespace jive
{
    // Roots are usually static objects, so this can't be a global that might
    // be initialised after them.
    [[nodiscard]] static std::vector<const SourceRoot*>& getSourceRoots()
    {
        static std::vector<const SourceRoot*> roots;
        return roots;
    }

    SourceRoot::SourceRoot(const juce::File& rootDirectory)
        : directory{ rootDirectory }
    {
        getSourceRoots().push_back(this);
    }

    SourceRoot::SourceRoot(const void* zipDataToUse, std::size_t zipSizeInBytes)
        : zipData{ zipDataToUse }
        , zipSize{ zipSizeInBytes }
    {
        getSourceRoots().push_back(this);
    }

    SourceRoot::~SourceRoot()
    {
        auto& roots = getSourceRoots();
        roots.erase(std::remove(std::begin(roots), std::end(roots), this), std::end(roots));
    }

    const SourceRoot* SourceRoot::find(const juce::String& relativePath)
    {
        for (const auto* root : getSourceRoots())
        {
            if (root->contains(relativePath))
                return root;
        }

        return nullptr;
    }

    bool SourceRoot::isEmbedded() const
    {
        return zipData != nullptr;
    }

    // A ZipFile only reads an entry while it's alive, so the entry is passed
    // to the callback rather than returned.
    template <typename Callback>
    static bool withZipEntry(const void* zipData,
                             std::size_t zipSize,
                             const juce::String& relativePath,
                             Callback&& callback)
    {
        juce::ZipFile zipFile{
#if JUCE_VERSION >= JIVE_JUCE_VERSION(8, 0, 13)
            std::make_unique<juce::MemoryInputStream>(zipData, zipSize, false),
#else
            new juce::MemoryInputStream{ zipData, zipSize, false },
            true,
#endif
        };

        if (const auto index = zipFile.getIndexOfFileName(relativePath,
                                                          !juce::File::areFileNamesCaseSensitive());
            index >= 0)
        {
            callback(zipFile, index);
            return true;
        }

        return false;
    }

    bool SourceRoot::contains(const juce::String& relativePath) const
    {
        if (isEmbedded())
            return withZipEntry(zipData, zipSize, relativePath, [](auto&, auto) {});

        return directory.getChildFile(relativePath).existsAsFile();
    }

    juce::File SourceRoot::getFileOnDisk(const juce::String& relativePath) const
    {
        if (isEmbedded())
            return {};

        return directory.getChildFile(relativePath);
    }

    juce::MemoryBlock SourceRoot::load(const juce::String& relativePath) const
    {
        juce::MemoryBlock data;

        if (isEmbedded())
        {
            withZipEntry(zipData, zipSize, relativePath, [&data](juce::ZipFile& zipFile, int index) {
                if (const std::unique_ptr<juce::InputStream> stream{ zipFile.createStreamForEntry(index) })
                    stream->readIntoMemoryBlock(data);
            });
        }
        else
        {
            directory.getChildFile(relativePath).loadFileAsData(data);
        }

        return data;
    }
} // namespace jive
