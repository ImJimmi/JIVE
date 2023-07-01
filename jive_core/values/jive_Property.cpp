#include <jive_layouts/jive_layouts.h>

#if JIVE_UNIT_TESTS
class PropertyUnitTest : public juce::UnitTest
{
public:
    PropertyUnitTest()
        : juce::UnitTest{ "jive::Property", "jive" }
    {
    }

    void runTest() final
    {
        testReading();
        testWriting();
        testCallback();
        testHereditaryValues();
        testObservations();
    }

private:
    void testReading()
    {
        beginTest("reading");

        juce::ValueTree tree{ "Tree", { { "value", 123 } } };
        jive::Property<int> value{ tree, "value" };

        expect(value == static_cast<int>(tree["value"]));

        tree.setProperty("value", 246, nullptr);

        expect(value == static_cast<int>(tree["value"]));
    }

    void testWriting()
    {
        beginTest("writing");

        juce::ValueTree tree{ "Tree", { { "value", "Some text" } } };
        jive::Property<juce::String> value{ tree, "value" };

        value = "Some different text";

        expect(tree["value"].toString() == "Some different text");

        value = "Even more different text";

        expect(tree["value"].toString() == "Even more different text");
    }

    void testCallback()
    {
        beginTest("callback");

        juce::ValueTree tree{ "Tree", { { "value", 1.23f } } };
        jive::Property<float> value{ tree, "value" };

        auto callbackCalled = false;
        value.onValueChange = [&callbackCalled]() {
            callbackCalled = true;
        };

        tree.setProperty("value", 2.46f, nullptr);

        expect(callbackCalled);
    }

    void testHereditaryValues()
    {
        beginTest("hereditary values");

        {
            juce::ValueTree root{
                "Root",
                {
                    { "value", 111 },
                },
                {
                    juce::ValueTree{
                        "Parent",
                        {
                            { "value", 222 },
                        },
                        {
                            juce::ValueTree{ "Child" },
                        },
                    },
                },
            };
            jive::Property<int, jive::Inheritance::inheritFromParent> value{
                root.getChild(0).getChild(0),
                "value",
            };
            expectEquals(value.get(), 222);

            root.setProperty("value", 999, nullptr);
            expectEquals(value.get(), 222);

            root.getChild(0).setProperty("value", 888, nullptr);
            expectEquals(value.get(), 888);

            root.getChild(0).getChild(0).setProperty("value", 777, nullptr);
            expectEquals(value.get(), 777);
        }
        {
            juce::ValueTree root{
                "Root",
                {
                    { "value", 111 },
                },
                {
                    juce::ValueTree{
                        "Parent",
                        {},
                        {
                            juce::ValueTree{ "Child" },
                        },
                    },
                },
            };
            jive::Property<int, jive::Inheritance::inheritFromAncestors> value{
                root.getChild(0).getChild(0),
                "value",
            };
            expectEquals(value.get(), 111);

            root.setProperty("value", 999, nullptr);
            expectEquals(value.get(), 999);

            root.getChild(0).setProperty("value", 888, nullptr);
            expectEquals(value.get(), 888);

            root.getChild(0).getChild(0).setProperty("value", 777, nullptr);
            expectEquals(value.get(), 777);
        }
        {
            juce::ValueTree root{
                "Root",
                {
                    { "value", 111 },
                },
                {
                    juce::ValueTree{
                        "Parent",
                        {},
                        {
                            juce::ValueTree{ "Child" },
                        },
                    },
                },
            };
            jive::Property<int, jive::Inheritance::doNotInherit> value{
                root.getChild(0).getChild(0),
                "value",
            };
            expectEquals(value.get(), 0);

            root.setProperty("value", 999, nullptr);
            expectEquals(value.get(), 0);

            root.getChild(0).setProperty("value", 888, nullptr);
            expectEquals(value.get(), 0);

            root.getChild(0).getChild(0).setProperty("value", 777, nullptr);
            expectEquals(value.get(), 777);
        }
    }

    void testObservations()
    {
        beginTest("observations");

        juce::ValueTree state{
            "State",
            {},
            {
                juce::ValueTree{
                    "Child",
                    {},
                    {
                        juce::ValueTree{
                            "Grandchild",
                            {},
                        },
                    },
                },
            }
        };

        {
            jive::Property<int> foo{ state, "foo" };

            bool callbackCalled = false;
            foo.onValueChange = [&callbackCalled]() {
                callbackCalled = true;
            };

            state.getChild(0).setProperty("foo", 999, nullptr);
            expect(!callbackCalled);

            callbackCalled = false;
            state.getChild(0).getChild(0).setProperty("foo", 647, nullptr);
            expect(!callbackCalled);

            callbackCalled = false;
            state.setProperty("foo", 123, nullptr);
            expect(callbackCalled);
        }
        {
            jive::Property<int,
                           jive::Inheritance::doNotInherit,
                           jive::Accumulation::accumulate>
                foo{ state, "foo" };

            bool callbackCalled = false;
            foo.onValueChange = [&callbackCalled]() {
                callbackCalled = true;
            };

            state.getChild(0).setProperty("foo", 378, nullptr);
            expect(callbackCalled);

            callbackCalled = false;
            state.getChild(0).getChild(0).setProperty("foo", 117, nullptr);
            expect(callbackCalled);

            callbackCalled = false;
            state.setProperty("foo", 302, nullptr);
            expect(callbackCalled);
        }
    }
};

static PropertyUnitTest propertyUnitTest;
#endif
