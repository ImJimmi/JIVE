#include <utilities/test_ComponentUtilities.h>

#include <catch2/catch_test_macros.hpp>
#include <jive/jive.h>

//======================================================================================================================
SCENARIO("GUI items can have children")
{
    GIVEN("a GUI item")
    {
        jive::GuiItem item{ test::createDummyComponent(), juce::ValueTree{ "Component" } };

        THEN("the item has no children")
        {
            REQUIRE(item.getNumChildren() == 0);
        }
        THEN("the item's component has no children")
        {
            REQUIRE(item.getComponent().getNumChildComponents() == 0);
        }

        WHEN("a child is added to the item")
        {
            item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                          juce::ValueTree{ "Component" }));

            THEN("the item has a single child")
            {
                REQUIRE(item.getNumChildren() == 1);
            }
            THEN("the item's component has a single child")
            {
                REQUIRE(item.getComponent().getNumChildComponents() == 1);
            }
        }
        WHEN("multiple children are added to the item")
        {
            item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                          juce::ValueTree{ "Component" }));

            THEN("the item has the expected number of children")
            {
                REQUIRE(item.getNumChildren() == 3);
            }
            THEN("the item's component has the same number of children")
            {
                REQUIRE(item.getComponent().getNumChildComponents() == 3);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items can have an ID")
{
    GIVEN("a GUI item")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        THEN("the item's ID is empty")
        {
            REQUIRE(item.getID().isNull());
        }
        AND_THEN("the ID of the item's component is empty")
        {
            REQUIRE(item.getComponent().getComponentID().isEmpty());
        }

        WHEN("the item's ID is changed")
        {
            tree.setProperty("id", "comp123", nullptr);

            THEN("the item's ID matches the one specified")
            {
                REQUIRE(item.getID() == juce::Identifier{ "comp123" });
            }
            AND_THEN("the ID of the item's component matches the one specified")
            {
                REQUIRE(item.getComponent().getComponentID() == "comp123");
            }
        }
    }
    GIVEN("a value-tree with a specified 'id' property")
    {
        juce::ValueTree tree{
            "Component",
            { { "id", "specified" } }
        };

        WHEN("a GUI item is constructed from the tree")
        {
            jive::GuiItem item{ test::createDummyComponent(), tree };

            THEN("the item's ID matches the one in the tree")
            {
                REQUIRE(item.getID() == juce::Identifier{ tree["id"] });
            }
            AND_THEN("the ID of the item's component matches the one in the tree")
            {
                REQUIRE(item.getComponent().getComponentID() == tree["id"].toString());
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items have a width and height")
{
    GIVEN("a GUI item")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        THEN("the item has an auto width and height")
        {
            REQUIRE(item.hasAutoWidth());
            REQUIRE(item.hasAutoHeight());
        }
        THEN("the item's component should have a size of 0x0")
        {
            REQUIRE(item.getComponent().getWidth() == 0.f);
            REQUIRE(item.getComponent().getHeight() == 0.f);
        }

        WHEN("the item's size is changed")
        {
            tree.setProperty("width", 100.11f, nullptr);
            tree.setProperty("height", 50.55f, nullptr);

            THEN("the item has the specified size")
            {
                REQUIRE(item.getWidth() == 100.11f);
                REQUIRE(item.getHeight() == 50.55f);
            }
            THEN("the item's component has the specified size (rounded to the nearest whole-number)")
            {
                REQUIRE(item.getComponent().getWidth() == 100);
                REQUIRE(item.getComponent().getHeight() == 51);
            }
        }
        WHEN("the item's padding and border width are changed")
        {
            tree.setProperty("padding", 10, nullptr);
            tree.setProperty("border-width", 5, nullptr);

            AND_WHEN("the item's size is changed")
            {
                tree.setProperty("width", 150, nullptr);
                tree.setProperty("height", 100, nullptr);

                THEN("the item has the specified size")
                {
                    REQUIRE(item.getWidth() == 150.f);
                    REQUIRE(item.getHeight() == 100.f);
                }
                THEN("the item's component has the specified size plus the size of the padding and border")
                {
                    REQUIRE(item.getComponent().getWidth() == 180);
                    REQUIRE(item.getComponent().getHeight() == 130);
                }
            }
        }
    }
}
