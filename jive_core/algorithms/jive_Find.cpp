#include "jive_Find.h"

#include <jive_core/values/variant-converters/jive_VariantConvertion.h>

namespace jive
{
    juce::ValueTree find(const juce::ValueTree& root,
                         std::function<bool(const juce::ValueTree&)> predicate)
    {
        if (predicate == nullptr)
            return {};

        if (predicate(root))
            return root;

        for (auto child : root)
        {
            if (predicate(child))
                return child;
        }

        for (auto child : root)
        {
            if (auto result = find(child, predicate);
                result != juce::ValueTree{})
            {
                return result;
            }
        }

        return {};
    }

    std::vector<juce::ValueTree> findAll(const juce::ValueTree& root,
                                         std::function<bool(const juce::ValueTree&)> predicate)
    {
        if (predicate == nullptr)
            return {};

        std::vector<juce::ValueTree> result;

        if (predicate(root))
            result.emplace_back(root);

        for (auto child : root)
        {
            auto others = findAll(child, predicate);
            result.insert(std::end(result), std::begin(others), std::end(others));
        }

        return result;
    }

    juce::ValueTree findElementWithID(const juce::ValueTree& root,
                                      const juce::Identifier& id)
    {
        return find(root,
                    [id](const auto& element) {
                        return fromVar<juce::Identifier>(element["id"]) == id;
                    });
    }

    std::vector<juce::ValueTree> findElementsWithClass(const juce::ValueTree& root,
                                                       const juce::String& className)
    {
        return findAll(root, [&className](const auto& tree) {
            return juce::StringArray::fromTokens(tree["class"].toString(), true).contains(className);
        });
    }

    std::vector<juce::ValueTree> findElementsWithText(const juce::ValueTree& root,
                                                      const juce::String& text)
    {
        return findElementsWithText(std::vector{ root }, text);
    }

    std::vector<juce::ValueTree> findElementsWithText(const std::vector<juce::ValueTree>& roots,
                                                      const juce::String& text)
    {
        std::vector<juce::ValueTree> result;

        for (const auto& root : roots)
        {
            auto others = findAll(root, [&text](const auto& tree) {
                auto child = tree.getChildWithName("Text");
                return child.isValid() && child["text"].toString() == text;
            });
            result.insert(std::end(result), std::begin(others), std::end(others));
        }

        return result;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_core/logging/jive_StringStreams.h>

class FindUnitTest : public juce::UnitTest
{
public:
    FindUnitTest()
        : juce::UnitTest{ "jive::find()", "jive" }
    {
    }

    void runTest() final
    {
        testFind();
        testFindElementWithID();
    }

private:
    void testFind()
    {
        beginTest("jive::find()");

        juce::ValueTree root{
            "Root",
            {},
            {
                juce::ValueTree{ "Child1" },
                juce::ValueTree{
                    "Child2",
                    {
                        { "foo", 123 },
                        { "bar", 456 },
                    },
                    {
                        juce::ValueTree{
                            "Grandchild",
                            {
                                { "foo", 123 },
                                { "bar", 789 },
                            },
                        },
                    },
                },
            },
        };

        expectEquals(jive::find(root,
                                [](const auto& tree) {
                                    return tree.getType().toString() == "Root";
                                }),
                     root);
        expectEquals(jive::find(root,
                                [](const auto& tree) {
                                    return tree.getType().toString() == "Child1";
                                }),
                     root.getChild(0));
        expectEquals(jive::find(root,
                                [](const auto& tree) {
                                    return tree.hasProperty("foo");
                                }),
                     root.getChild(1));
        expectEquals(jive::find(root,
                                [](const auto& tree) {
                                    return static_cast<int>(tree["bar"]) == 789;
                                }),
                     root.getChild(1).getChild(0));
    }

    void testFindElementWithID()
    {
        beginTest("jive::findElementWithID()");

        juce::ValueTree state{
            "Foo",
            {
                { "id", "foo" },
            },
            {
                juce::ValueTree{
                    "Bar",
                    {
                        { "id", "bar" },
                    },
                },
            },
        };
        expectEquals(jive::findElementWithID(state, "foo"), state);
        expectEquals(jive::findElementWithID(state, "bar"), state.getChild(0));
    }
};

static FindUnitTest findUnitTest;
#endif
