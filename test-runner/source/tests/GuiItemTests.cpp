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
