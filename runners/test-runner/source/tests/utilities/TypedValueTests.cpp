#include <catch2/catch_test_macros.hpp>
#include <jive/jive.h>

//======================================================================================================================
SCENARIO("typed-values can be used to read a property from a value-tree")
{
    GIVEN("a value-tree with an integer property")
    {
        juce::ValueTree tree{
            "Tree",
            { { "value", 123 } }
        };

        WHEN("a typed-value is created for the tree's integer property")
        {
            jive::TypedValue<int> value{ tree, "value" };

            THEN("the typed-value's value should match the tree's integer property")
            {
                REQUIRE(static_cast<int>(value) == static_cast<int>(tree["value"]));
            }

            AND_WHEN("the tree's integer property is changed")
            {
                tree.setProperty("value", 246, nullptr);

                THEN("the typed-value's value should match the tree's updated integer property")
                {
                    REQUIRE(static_cast<int>(value) == static_cast<int>(tree["value"]));
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("typed-values can be used to modify a value-tree's property")
{
    GIVEN("a value-tree with a string property")
    {
        juce::ValueTree tree{
            "Tree",
            { { "value", "Some text" } }
        };

        WHEN("a typed-value is created for the tree's string property")
        {
            jive::TypedValue<juce::String> value{ tree, "value" };

            AND_WHEN("the typed-value's value is changed")
            {
                value = "Some different text";

                THEN("the value-tree's string property matches the typed-value's value")
                {
                    REQUIRE(tree["value"].toString() == "Some different text");
                }
            }
            AND_WHEN("the typed-value's value is changed to something different")
            {
                value = "Even more different text";

                THEN("the value-tree's string property matches the typed-value's value")
                {
                    REQUIRE(tree["value"].toString() == "Even more different text");
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("typed-values have a callback that's called when the referenced value changes")
{
    GIVEN("a value-tree with a float property")
    {
        juce::ValueTree tree{
            "Tree",
            { { "value", 1.23f } }
        };

        WHEN("a typed-value is created for the tree's float property")
        {
            auto callbackCalled = false;
            jive::TypedValue<float> value{ tree, "value" };
            value.onValueChange = [&callbackCalled]() {
                callbackCalled = true;
            };

            AND_WHEN("the tree's float property is changed")
            {
                callbackCalled = false;
                tree.setProperty("value", 2.46f, nullptr);

                THEN("the typed-value's callback was called")
                {
                    REQUIRE(callbackCalled);
                }
            }
            AND_WHEN("the tree's float property is set to its current value")
            {
                callbackCalled = false;
                tree.setProperty("value", 1.23f, nullptr);

                THEN("the typed-value's callback was not called")
                {
                    REQUIRE_FALSE(callbackCalled);
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("type-values can have an initial value if the property they're asked to refer to doesn't exist")
{
    GIVEN("a value-tree with no properties")
    {
        juce::ValueTree tree{ "Tree" };

        WHEN("a typed-value is created for a non-existent property in the tree, initialised to some value")
        {
            jive::TypedValue<int> value{ tree, "valueThatDoesntExist", 987 };

            THEN("the typed-value's value is equal to the initialised value")
            {
                REQUIRE(value == 987);
            }
        }
        WHEN("a property is set on the tree")
        {
            tree.setProperty("value", 574, nullptr);

            AND_WHEN("a typed-value is created for the value, with a specified initial value that's different to the "
                     "value of the tree's property")
            {
                jive::TypedValue<int> value{ tree, "value", 9226 };

                THEN("the typed-value's value matches the value originally set in the tree")
                {
                    REQUIRE(value == 574);
                }
            }
        }
    }
}
