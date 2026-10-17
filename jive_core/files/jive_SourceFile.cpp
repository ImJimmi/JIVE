#include "jive_SourceFile.h"

#include "jive_EmbeddedSources.h"

namespace jive
{
    SourceFile::SourceFile(const juce::File& sourceFile)
        : file{ sourceFile }
    {
    }

    const juce::File& SourceFile::getFile() const
    {
        return file;
    }

    bool SourceFile::isEmbedded() const
    {
        return EmbeddedSources::isEmbedded(file);
    }

    bool SourceFile::exists() const
    {
        if (isEmbedded())
            return EmbeddedSources::contains(file);

        return file.existsAsFile();
    }

    juce::String SourceFile::loadAsString() const
    {
        if (isEmbedded())
        {
            const auto data = loadAsData();
            return juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
        }

        return file.loadFileAsString();
    }

    juce::MemoryBlock SourceFile::loadAsData() const
    {
        if (isEmbedded())
            return EmbeddedSources::load(file).value_or(juce::MemoryBlock{});

        juce::MemoryBlock data;
        file.loadFileAsData(data);
        return data;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include "jive_SourceDirectories.h"

    #include <jive_core/testing/jive_TestZip.h>
    #include <jive_core/testing/jive_UnitTest.h>

static jive::UnitTest embeddedSourceFilesTest{
    "jive",
    "jive::SourceFile",
    "Files in an embedded directory are read from its zip, never from disk",
    [](auto& test) {
        const auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
                              .getChildFile("jive-embedded-source-file-tests");
        root.deleteRecursively();
        root.getChildFile("views").createDirectory();
        root.getChildFile("views/both.xml").replaceWithText("<Disk/>");
        root.getChildFile("views/disk-only.xml").replaceWithText("<Disk/>");

        const auto zip = jive::createZip({
            { "views/both.xml", "<Embedded/>" },
            { "views/embedded-only.xml", "<Embedded/>" },
        });

        {
            const jive::EmbeddedSources::Registration registration{ root, zip.getData(), zip.getSize() };

            const jive::SourceFile both{ root.getChildFile("views/both.xml") };
            test.expect(both.isEmbedded());
            test.expect(both.exists());
            test.expectEquals(both.loadAsString(), juce::String{ "<Embedded/>" });

            const jive::SourceFile embeddedOnly{ root.getChildFile("views/embedded-only.xml") };
            test.expect(embeddedOnly.exists());
            test.expectEquals(embeddedOnly.loadAsString(), juce::String{ "<Embedded/>" });

            const jive::SourceFile diskOnly{ root.getChildFile("views/disk-only.xml") };
            test.expect(!diskOnly.exists());
            test.expectEquals(diskOnly.loadAsString(), juce::String{});

            const jive::SourceDirectories directories{ root.getChildFile("views") };
            test.expect(directories.find("embedded-only.xml").exists());
            test.expect(!directories.find("disk-only.xml").exists());
        }

        const jive::SourceFile both{ root.getChildFile("views/both.xml") };
        test.expect(!both.isEmbedded(), "Files stop being embedded once their registration is gone");
        test.expectEquals(both.loadAsString(), juce::String{ "<Disk/>" });

        root.deleteRecursively();
    },
};
#endif
