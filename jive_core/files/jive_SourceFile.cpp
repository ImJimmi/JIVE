#include "jive_SourceFile.h"

#include "jive_SourceRoot.h"

#include <jive_core/values/jive_Identifiers.h>

namespace jive
{
    // Zip entries are always separated by forward slashes, and paths that only
    // differ by "." or ".." need to compare equal.
    [[nodiscard]] static juce::String normaliseRelativePath(const juce::String& path)
    {
        juce::StringArray parts;

        for (const auto& part : juce::StringArray::fromTokens(path.replaceCharacter('\\', '/'), "/", ""))
        {
            if (part.isEmpty() || part == ".")
                continue;

            if (part == ".." && !parts.isEmpty() && parts[parts.size() - 1] != "..")
                parts.remove(parts.size() - 1);
            else
                parts.add(part);
        }

        return parts.joinIntoString("/");
    }

    SourceFile::SourceFile(const juce::String& relativeOrAbsolutePath)
        : path{ juce::File::isAbsolutePath(relativeOrAbsolutePath)
                    ? juce::File{ relativeOrAbsolutePath }.getFullPathName()
                    : normaliseRelativePath(relativeOrAbsolutePath) }
    {
    }

    SourceFile::SourceFile(const juce::File& file)
        : path{ file.getFullPathName() }
    {
    }

    const juce::String& SourceFile::getPath() const
    {
        return path;
    }

    SourceFile SourceFile::getChildFile(const juce::String& relativePath) const
    {
        if (juce::File::isAbsolutePath(relativePath))
            return SourceFile{ relativePath };

        if (isAbsolute())
            return SourceFile{ juce::File{ path }.getChildFile(relativePath) };

        return SourceFile{ path.isEmpty() ? relativePath : path + "/" + relativePath };
    }

    SourceFile SourceFile::getParentDirectory() const
    {
        if (isAbsolute())
            return SourceFile{ juce::File{ path }.getParentDirectory() };

        return SourceFile{ path.containsChar('/') ? path.upToLastOccurrenceOf("/", false, false) : juce::String{} };
    }

    juce::File SourceFile::getFileOnDisk() const
    {
        if (isAbsolute())
            return juce::File{ path };

        if (const auto* root = SourceRoot::find(path))
            return root->getFileOnDisk(path);

        return {};
    }

    bool SourceFile::isEmbedded() const
    {
        if (isAbsolute())
            return false;

        const auto* root = SourceRoot::find(path);
        return root != nullptr && root->isEmbedded();
    }

    bool SourceFile::exists() const
    {
        if (isAbsolute())
            return juce::File{ path }.existsAsFile();

        return SourceRoot::find(path) != nullptr;
    }

    juce::String SourceFile::loadAsString() const
    {
        const auto data = loadAsData();
        return juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
    }

    juce::MemoryBlock SourceFile::loadAsData() const
    {
        if (isAbsolute())
        {
            juce::MemoryBlock data;
            juce::File{ path }.loadFileAsData(data);
            return data;
        }

        if (const auto* root = SourceRoot::find(path))
            return root->load(path);

        return {};
    }

    bool SourceFile::operator==(const SourceFile& other) const
    {
        return path == other.path;
    }

    bool SourceFile::operator!=(const SourceFile& other) const
    {
        return !(*this == other);
    }

    bool SourceFile::isAbsolute() const
    {
        return juce::File::isAbsolutePath(path);
    }

    SourceFile findSourceFile(const juce::ValueTree& tree)
    {
        for (auto ancestor = tree; ancestor.isValid(); ancestor = ancestor.getParent())
        {
            if (ancestor.hasProperty(ids::sourceFile()))
                return SourceFile{ ancestor[ids::sourceFile()].toString() };
        }

        return {};
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include "jive_SourceDirectories.h"

    #include <jive_core/testing/jive_TestZip.h>
    #include <jive_core/testing/jive_UnitTest.h>

static jive::UnitTest relativeSourceFilesTest{
    "jive",
    "jive::SourceFile",
    "Relative paths are found in the roots created by jive_sources()",
    [](auto& test) {
        const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory)
                                   .getChildFile("jive-source-file-tests");
        directory.deleteRecursively();
        directory.getChildFile("views").createDirectory();
        directory.getChildFile("views/both.xml").replaceWithText("<Disk/>");
        directory.getChildFile("views/disk-only.xml").replaceWithText("<Disk/>");

        const auto zip = jive::createZip({
            { "views/both.xml", "<Embedded/>" },
            { "views/embedded-only.xml", "<Embedded/>" },
        });

        test.expect(!jive::SourceFile{ "views/both.xml" }.exists(), "Relative paths can't be found without a root");

        {
            const jive::SourceRoot embedded{ zip.getData(), zip.getSize() };
            const jive::SourceRoot onDisk{ directory };

            const jive::SourceFile both{ "views/both.xml" };
            test.expect(both.isEmbedded(), "Roots should be searched in the order they were created");
            test.expectEquals(both.loadAsString(), juce::String{ "<Embedded/>" });
            test.expect(both.getFileOnDisk() == juce::File{});

            const jive::SourceFile diskOnly{ "views/disk-only.xml" };
            test.expect(diskOnly.exists());
            test.expect(!diskOnly.isEmbedded());
            test.expect(diskOnly.getFileOnDisk() == directory.getChildFile("views/disk-only.xml"));
            test.expectEquals(diskOnly.loadAsString(), juce::String{ "<Disk/>" });

            test.expectEquals(jive::SourceFile{ "views/embedded-only.xml" }.loadAsString(), juce::String{ "<Embedded/>" });
            test.expect(!jive::SourceFile{ "views/missing.xml" }.exists());

            const jive::SourceDirectories directories{ jive::SourceFile{ "views" } };
            test.expect(directories.find("embedded-only.xml").exists());
            test.expect(directories.find("disk-only.xml").exists());
        }

        test.expect(!jive::SourceFile{ "views/embedded-only.xml" }.exists(),
                    "Roots should stop being searched once they're destroyed");

        directory.deleteRecursively();
    },
};

static jive::UnitTest absoluteSourceFilesTest{
    "jive",
    "jive::SourceFile",
    "Absolute paths are read from disk",
    [](auto& test) {
        const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory);
        const auto file = directory.getChildFile("jive-absolute-source-file-test.xml");
        file.replaceWithText("<Disk/>");

        const auto zip = jive::createZip({ { "jive-absolute-source-file-test.xml", "<Embedded/>" } });
        const jive::SourceRoot embedded{ zip.getData(), zip.getSize() };

        const jive::SourceFile sourceFile{ file };
        test.expect(sourceFile.exists());
        test.expect(!sourceFile.isEmbedded());
        test.expect(sourceFile.getFileOnDisk() == file);
        test.expectEquals(sourceFile.loadAsString(), juce::String{ "<Disk/>" });

        file.deleteFile();
    },
};

static jive::UnitTest sourceFilePathsTest{
    "jive",
    "jive::SourceFile",
    "Paths are normalised",
    [](auto& test) {
        test.expectEquals(jive::SourceFile{ "views/./icons/../Main.xml" }.getPath(), juce::String{ "views/Main.xml" });
        test.expectEquals(jive::SourceFile{ "views\\Main.xml" }.getPath(), juce::String{ "views/Main.xml" });
        test.expectEquals(jive::SourceFile{ "views" }.getChildFile("../shared/Styles.json").getPath(),
                          juce::String{ "shared/Styles.json" });
        test.expectEquals(jive::SourceFile{ "views/Main.xml" }.getParentDirectory().getPath(), juce::String{ "views" });
        test.expectEquals(jive::SourceFile{ "Main.xml" }.getParentDirectory().getChildFile("Styles.json").getPath(),
                          juce::String{ "Styles.json" });

        const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory);
        test.expectEquals(jive::SourceFile{ directory }.getChildFile("Main.xml").getPath(),
                          directory.getChildFile("Main.xml").getFullPathName());
    },
};
#endif

#if JIVE_UNIT_TESTS
static jive::UnitTest sourceDirectoriesReferrerTest{
    "jive",
    "jive::SourceDirectories",
    "Paths starting with ./ or ../ are relative to the file that refers to them",
    [](auto& test) {
        const auto zip = jive::createZip({
            { "Styles.json", "root" },
            { "views/Styles.json", "views" },
            { "shared/Styles.json", "shared" },
        });
        const jive::SourceRoot root{ zip.getData(), zip.getSize() };
        const jive::SourceDirectories directories{ jive::SourceFile{} };
        const jive::SourceFile referrer{ "views/Main.xml" };

        test.expectEquals(directories.find("Styles.json", referrer).getPath(), juce::String{ "Styles.json" });
        test.expectEquals(directories.find("./Styles.json", referrer).getPath(), juce::String{ "views/Styles.json" });
        test.expectEquals(directories.find("../shared/Styles.json", referrer).getPath(), juce::String{ "shared/Styles.json" });
        test.expect(!directories.find("./Missing.json", referrer).exists());
    },
};

static jive::UnitTest findSourceFileTest{
    "jive",
    "jive::SourceFile",
    "A tree's source file is the one it, or its closest ancestor, was parsed from",
    [](auto& test) {
        juce::ValueTree grandchild{ "Text" };
        juce::ValueTree child{ "Component", { { "jive::source-file", "views/Child.xml" } }, { grandchild } };
        juce::ValueTree root{ "Window", { { "jive::source-file", "views/Main.xml" } }, { child } };

        test.expectEquals(jive::findSourceFile(root).getPath(), juce::String{ "views/Main.xml" });
        test.expectEquals(jive::findSourceFile(grandchild).getPath(), juce::String{ "views/Child.xml" });
        test.expectEquals(jive::findSourceFile(juce::ValueTree{ "Text" }).getPath(), juce::String{});
    },
};
#endif
