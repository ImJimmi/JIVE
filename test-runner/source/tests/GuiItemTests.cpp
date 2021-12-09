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
SCENARIO("GUI items can have a padding")
{
    GIVEN("a GUI item")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ test::createDummyComponent(), tree };
        item.getComponent().setSize(100, 100);

        THEN("the item has no padding")
        {
            REQUIRE(item.getPadding() == juce::BorderSize<float>{ 0.f });
        }
        THEN("the item's content bounds has no padding applied")
        {
            REQUIRE(item.getContentBounds() == juce::Rectangle<float>{ 0, 0, 100, 100 });
        }

        WHEN("the item's padding is set to a single numeric value")
        {
            tree.setProperty("padding", 5, nullptr);

            THEN("the item's padding has the specified value on all sides")
            {
                REQUIRE(item.getPadding() == juce::BorderSize<float>{ 5.f });
            }
            THEN("the item's content bounds have the padding applied")
            {
                REQUIRE(item.getContentBounds() == juce::Rectangle<float>{ 5, 5, 90, 90 });
            }
        }
        WHEN("the item's padding is set to two numeric values")
        {
            tree.setProperty("padding", "5 10", nullptr);

            THEN("the item's padding matches the first value for its top and bottom values")
            {
                REQUIRE(item.getPadding().getTop() == 5.f);
                REQUIRE(item.getPadding().getBottom() == 5.f);
            }
            THEN("the item's padding matches the second value for its left and right values")
            {
                REQUIRE(item.getPadding().getLeft() == 10.f);
                REQUIRE(item.getPadding().getRight() == 10.f);
            }
            THEN("the item's content bounds have the padding applied")
            {
                REQUIRE(item.getContentBounds() == juce::Rectangle<float>{ 10, 5, 80, 90 });
            }
        }
        WHEN("the item's padding is set to three numeric values")
        {
            tree.setProperty("padding", "3 6 9", nullptr);

            THEN("the item's padding matches the first value for its top value")
            {
                REQUIRE(item.getPadding().getTop() == 3.f);
            }
            THEN("the item's padding matches the second value for its left and right values")
            {
                REQUIRE(item.getPadding().getLeft() == 6.f);
                REQUIRE(item.getPadding().getRight() == 6.f);
            }
            THEN("the item's padding matches the third value for its bottom value")
            {
                REQUIRE(item.getPadding().getBottom() == 9.f);
            }
            THEN("the item's content bounds have the padding applied")
            {
                REQUIRE(item.getContentBounds() == juce::Rectangle<float>{ 6, 3, 88, 88 });
            }
        }
        WHEN("the item's padding is set to four numeric values")
        {
            tree.setProperty("padding", "2 4 6 8", nullptr);

            THEN("the item's padding matches the first value for its top value")
            {
                REQUIRE(item.getPadding().getTop() == 2.f);
            }
            THEN("the item's padding matches the second value for its right value")
            {
                REQUIRE(item.getPadding().getRight() == 4.f);
            }
            THEN("the item's padding matches the third value for its bottom value")
            {
                REQUIRE(item.getPadding().getBottom() == 6.f);
            }
            THEN("the item's padding matches the forth value for its left value")
            {
                REQUIRE(item.getPadding().getLeft() == 8.f);
            }
            THEN("the item's content bounds have the padding applied")
            {
                REQUIRE(item.getContentBounds() == juce::Rectangle<float>{ 8, 2, 88, 92 });
            }
        }
    }
}
