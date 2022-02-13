#include "jive_TypedValue.h"

//======================================================================================================================
#if JIVE_UNIT_TESTS
class TypedValueUnitTest : public juce::UnitTest
{
public:
    TypedValueUnitTest()
        : juce::UnitTest{ "jive::TypedValue", "jive" }
    {
    }

    void runTest() final
    {
        testReading();
        testWriting();
        testCallback();
        testInitialValue();
    }

private:
    void testReading()
    {
        beginTest("reading");

        juce::ValueTree tree{ "Tree", { { "value", 123 } } };
        jive::TypedValue<int> value{ tree, "value" };

        expect(value == static_cast<int>(tree["value"]));

        tree.setProperty("value", 246, nullptr);

        expect(value == static_cast<int>(tree["value"]));
    }

    void testWriting()
    {
        beginTest("writing");

        juce::ValueTree tree{ "Tree", { { "value", "Some text" } } };
        jive::TypedValue<juce::String> value{ tree, "value" };

        value = "Some different text";

        expect(tree["value"].toString() == "Some different text");

        value = "Even more different text";

        expect(tree["value"].toString() == "Even more different text");
    }

    void testCallback()
    {
        beginTest("callback");

        juce::ValueTree tree{ "Tree", { { "value", 1.23f } } };
        jive::TypedValue<float> value{ tree, "value" };

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
            jive::TypedValue<int> value{ tree, "valueThatDoesntExist", 987 };

            expect(value == 987);
        }
        {
            juce::ValueTree tree{ "Tree", { { "value", 574 } } };
            jive::TypedValue<int> value{ tree, "value", 9226 };

            expect(value == 574);
        }
    }
};

static TypedValueUnitTest typedValueUnitTest;
#endif
