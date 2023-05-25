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
        testInitialValue();
        testHereditaryValues();
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

    void testInitialValue()
    {
        beginTest("initial value");

        {
            juce::ValueTree tree{ "Tree" };
            jive::Property<int> value{ tree, "valueThatDoesntExist", 987 };

            expect(value == 987);
        }
        {
            juce::ValueTree tree{ "Tree", { { "value", 574 } } };
            jive::Property<int> value{ tree, "value", 9226 };

            expect(value == 574);
        }
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
            jive::Property<int, jive::HereditaryValueBehaviour::inheritFromParent> value{
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
            jive::Property<int, jive::HereditaryValueBehaviour::inheritFromAncestors> value{
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
            jive::Property<int, jive::HereditaryValueBehaviour::doNotInherit> value{
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
};

static PropertyUnitTest propertyUnitTest;
#endif
