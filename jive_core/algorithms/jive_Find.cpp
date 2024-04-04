#include "jive_Find.h"

namespace jive
{
    juce::ValueTree find(const juce::ValueTree& root,
                         std::function<bool(const juce::ValueTree&)> predicate)
    {
        if (predicate == nullptr)
            return {};

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
} // namespace jive

#if JIVE_UNIT_TESTS
struct FindUnitTest : public juce::UnitTest
{
    FindUnitTest()
        : juce::UnitTest{ "jive::find()", "jive" }
    {
    }

    void runTest() final
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

        expect(jive::find(root,
                          [](const auto& tree) {
                              return tree.getType().toString() == "Root";
                          })
               == juce::ValueTree{});
        expect(jive::find(root,
                          [](const auto& tree) {
                              return tree.getType().toString() == "Child1";
                          })
               == root.getChild(0));
        expect(jive::find(root,
                          [](const auto& tree) {
                              return tree.hasProperty("foo");
                          })
               == root.getChild(1));
        expect(jive::find(root,
                          [](const auto& tree) {
                              return static_cast<int>(tree["bar"]) == 789;
                          })
               == root.getChild(1).getChild(0));
    }
};

static FindUnitTest findUnitTest;
#endif
