#include "jive_EmbeddedSources.h"

namespace jive
{
    struct EmbeddedZip
    {
        juce::File rootDirectory;
        const void* data;
        std::size_t size;
        const EmbeddedSources::Registration* registration;
    };

    // Registrations are usually static objects, so this can't be a global
    // that might be initialised after them.
    [[nodiscard]] static std::vector<EmbeddedZip>& getEmbeddedZips()
    {
        static std::vector<EmbeddedZip> zips;
        return zips;
    }

    EmbeddedSources::Registration::Registration(const juce::File& rootDirectory, const void* zipData, std::size_t zipSize)
    {
        getEmbeddedZips().push_back({ rootDirectory, zipData, zipSize, this });
    }

    EmbeddedSources::Registration::~Registration()
    {
        auto& zips = getEmbeddedZips();
        zips.erase(std::remove_if(std::begin(zips),
                                  std::end(zips),
                                  [this](const auto& zip) {
                                      return zip.registration == this;
                                  }),
                   std::end(zips));
    }

    bool EmbeddedSources::isEmbedded(const juce::File& file)
    {
        const auto& zips = getEmbeddedZips();
        return std::any_of(std::begin(zips),
                           std::end(zips),
                           [&file](const auto& zip) {
                               return file.isAChildOf(zip.rootDirectory);
                           });
    }

    // A ZipFile only reads an entry while it's alive, so the entry is passed
    // to the callback rather than returned.
    template <typename Callback>
    static bool withEntry(const juce::File& file, Callback&& callback)
    {
        for (const auto& zip : getEmbeddedZips())
        {
            if (!file.isAChildOf(zip.rootDirectory))
                continue;

            juce::ZipFile zipFile{ new juce::MemoryInputStream{ zip.data, zip.size, false }, true };
            const auto path = file.getRelativePathFrom(zip.rootDirectory).replaceCharacter('\\', '/');

            if (const auto index = zipFile.getIndexOfFileName(path, !juce::File::areFileNamesCaseSensitive());
                index >= 0)
            {
                callback(zipFile, index);
                return true;
            }
        }

        return false;
    }

    bool EmbeddedSources::contains(const juce::File& file)
    {
        return withEntry(file, [](auto&, auto) {});
    }

    std::optional<juce::MemoryBlock> EmbeddedSources::load(const juce::File& file)
    {
        std::optional<juce::MemoryBlock> result;

        withEntry(file, [&result](juce::ZipFile& zipFile, int index) {
            if (const std::unique_ptr<juce::InputStream> stream{ zipFile.createStreamForEntry(index) })
            {
                juce::MemoryBlock data;
                stream->readIntoMemoryBlock(data);
                result = std::move(data);
            }
        });

        return result;
    }
} // namespace jive
