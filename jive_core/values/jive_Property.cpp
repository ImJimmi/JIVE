#include "jive_Property.h"

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
        testFunctionalProperties();
        testDynamicObjectSource();
        testTransitions();
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
        {
            juce::ValueTree tree{
                "Foo",
                {},
                {
                    juce::ValueTree{ "Bar", { { "value", 12 } } },
                    juce::ValueTree{ "Bar", { { "value", 38 } } },
                },
            };
            const jive::Property<int,
                                 jive::Inheritance::doNotInherit,
                                 jive::Accumulation::accumulate>
                value{ tree, "value" };
            expectEquals(value.get(), 50);
        }
    }

    void testFunctionalProperties()
    {
        beginTest("functional properties");

        juce::ValueTree state{ "State" };
        jive::Property<int> value{ state, "value" };
        expect(!value.isFunctional());

        value = [] {
            return 300;
        };
        expect(value.isFunctional());
        expectEquals(value.get(), 300);
    }

    void testDynamicObjectSource()
    {
        juce::ValueTree state{
            "State",
            {
                {
                    "object",
                    new jive::Object{
                        { "value", 12345 },
                        {
                            "inner",
                            new jive::Object{
                                { "leaf", new jive::Object{} },
                            },
                        },
                    },
                },
            },
        };

        beginTest("DynamicObject source - standard template args");
        {
            jive::Property<int> value{
                dynamic_cast<jive::Object*>(state["object"].getObject()),
                "value",
            };
            expectEquals(value.get(), 12345);

            value.set(98765);
            expectEquals(value.get(), 98765);

            expect(!value.isAuto());
            value.setAuto();
            expect(value.isAuto());

            expect(value.exists());
            value.clear();
            expect(!value.exists());

            expect(!value.isFunctional());
            value = []() {
                return 999;
            };
            expect(value.isFunctional());
            expectEquals(value.get(), 999);

            value = 7648;
            expectEquals<juce::String>(value.toString(), "7648");

            bool receivedCallback = false;
            value.onValueChange = [&receivedCallback]() {
                receivedCallback = true;
            };
            state["object"].getDynamicObject()->setProperty("foo", 111);
            expect(!receivedCallback);
            state["object"].getDynamicObject()->setProperty("value", 11358);
            expect(receivedCallback);
        }

        beginTest("DynamicObject source - inheritFromParent");
        {
            jive::Property<int, jive::Inheritance::inheritFromParent> value{
                dynamic_cast<jive::Object*>(state["object"]["inner"].getObject()),
                "value",
            };
            expect(!value.exists());
            expectEquals(value.get(), static_cast<int>(state["object"]["value"]));

            value = 24816;
            expectEquals(value.get(), 24816);

            value.clear();
            expectEquals(value.get(), static_cast<int>(state["object"]["value"]));

            bool receivedCallback = false;
            value.onValueChange = [&receivedCallback]() {
                receivedCallback = true;
            };
            state["object"]["inner"].getDynamicObject()->setProperty("value", 11358);
            expect(receivedCallback);
            receivedCallback = false;
            state["object"].getDynamicObject()->setProperty("value", 34544);
            expect(receivedCallback);
        }

        beginTest("DynamicObject source - inheritFromAncestors");
        {
            state["object"]["inner"].getDynamicObject()->removeProperty("value");
            jive::Property<int, jive::Inheritance::inheritFromAncestors> value{
                dynamic_cast<jive::Object*>(state["object"]["inner"]["leaf"].getObject()),
                "value",
            };
            expect(!value.exists());
            expectEquals(value.get(), static_cast<int>(state["object"]["value"]));
        }

        beginTest("DynamicObject source - accumulate");
        {
            state["object"].getDynamicObject()->setProperty("value", 123);
            state["object"]["inner"].getDynamicObject()->setProperty("value", 456);
            state["object"]["inner"]["leaf"].getDynamicObject()->setProperty("value", 789);

            jive::Property<int, jive::Inheritance::doNotInherit, jive::Accumulation::accumulate> value{
                dynamic_cast<jive::Object*>(state["object"].getObject()),
                "value",
            };
            expect(value.exists());
            expectEquals(value.get(),
                         static_cast<int>(state["object"]["value"])
                             + static_cast<int>(state["object"]["inner"]["value"])
                             + static_cast<int>(state["object"]["inner"]["leaf"]["value"]));
        }
    }

    void testTransitions()
    {
        beginTest("transitions / initialised");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "value", 0.0 },
                    { "transition", "value 2s" },
                },
            };
            jive::Property<double> value{ state, "value" };
            expectEquals(value.get(), 0.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 0.0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(value.get(), 0.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 0.0);

            value = 100.0;
            expectEquals(value.get(), 100.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 0.0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(value.get(), 100.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 50.0);

            jive::Property<double> other{ state, "value" };
            expectEquals(other.get(), 100.0);
            expectEquals(other.getTransition()->calculateCurrent<double>(), 50.0);

            value = 200.0;
            expectEquals(other.get(), 200.0);
            expectEquals(other.getTransition()->calculateCurrent<double>(), 50.0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(value.get(), 200.0);
            expectEquals(other.get(), 200.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 125.0);
            expectEquals(other.getTransition()->calculateCurrent<double>(), 125.0);
        }
        beginTest("transitions / uninitialised");
        {
            juce::ValueTree state{
                "Component",
                {
                    { "value", 0.0 },
                },
            };
            jive::Property<double> value{ state, "value" };
            expectEquals(value.get(), 0.0);
            expect(value.getTransition() == nullptr);

            value = 100.0;
            expectEquals(value.get(), 100.0);
            expect(value.getTransition() == nullptr);

            state.setProperty("transition", "value 1s", nullptr);
            value = 200;
            expectEquals(value.get(), 200.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 100.0);

            jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));
            expectEquals(value.get(), 200.0);
            expectEquals(value.getTransition()->calculateCurrent<double>(), 200.0);
        }
    }
};

static PropertyUnitTest propertyUnitTest;
#endif
