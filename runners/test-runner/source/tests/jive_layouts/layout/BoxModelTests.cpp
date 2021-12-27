#include <catch2/catch_test_macros.hpp>
#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
SCENARIO("box models have a width")
{
    GIVEN("a box model")
    {
        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        THEN("the box has a default width of -1")
        {
            REQUIRE(box.getWidth() == -1.f);
        }

        WHEN("the width is changed")
        {
            tree.setProperty("width", 312.4f, nullptr);

            THEN("the box has the new width")
            {
                REQUIRE(box.getWidth() == 312.4f);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("box models have a height")
{
    GIVEN("a box model")
    {
        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        THEN("the box model has a default height of -1")
        {
            REQUIRE(box.getHeight() == -1.f);
        }

        WHEN("the height is changed")
        {
            tree.setProperty("height", 846.2f, nullptr);

            THEN("the box has the new height")
            {
                REQUIRE(box.getHeight() == 846.2f);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("box models can have a padding")
{
    GIVEN("a box model")
    {
        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        THEN("the box has no padding")
        {
            REQUIRE(box.getPadding() == juce::BorderSize<float>{ 0.f });
        }

        WHEN("the padding is set with a single value")
        {
            tree.setProperty("padding", "14", nullptr);

            THEN("the box's padding on all sides matches the single value")
            {
                REQUIRE(box.getPadding() == juce::BorderSize<float>{ 14.f });
            }
        }
        AND_WHEN("the padding is set with two values")
        {
            tree.setProperty("padding", "112.4 73.7", nullptr);

            THEN("the top and bottom of the box's padding matches the first value")
            {
                REQUIRE(box.getPadding().getTop() == 112.4f);
                REQUIRE(box.getPadding().getBottom() == 112.4f);
            }
            AND_THEN("the left and right of the box's padding matches the second value")
            {
                REQUIRE(box.getPadding().getLeft() == 73.7f);
                REQUIRE(box.getPadding().getRight() == 73.7f);
            }
        }
        AND_WHEN("the padding is set with three values")
        {
            tree.setProperty("padding", "14.25 8.3 1.1", nullptr);

            THEN("the top of the box's padding matches the first value")
            {
                REQUIRE(box.getPadding().getTop() == 14.25f);
            }
            AND_THEN("the left and right of the box's padding matches the second value")
            {
                REQUIRE(box.getPadding().getLeft() == 8.3f);
                REQUIRE(box.getPadding().getRight() == 8.3f);
            }
            AND_THEN("the bottom of the box's padding matches the third value")
            {
                REQUIRE(box.getPadding().getBottom() == 1.1f);
            }
        }
        AND_WHEN("the padding is set with four values")
        {
            tree.setProperty("padding", "1 2 3 4", nullptr);

            THEN("the top of the box's padding matches the first value")
            {
                REQUIRE(box.getPadding().getTop() == 1.f);
            }
            AND_THEN("the right of the box's padding matches the second value")
            {
                REQUIRE(box.getPadding().getRight() == 2.f);
            }
            AND_THEN("the bottom of the box's padding matches the third value")
            {
                REQUIRE(box.getPadding().getBottom() == 3.f);
            }
            AND_THEN("the left of the box's padding matches the fourth value")
            {
                REQUIRE(box.getPadding().getLeft() == 4.f);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("box models can have a border")
{
    GIVEN("a box model")
    {
        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        THEN("the box has a zero-width border")
        {
            REQUIRE(box.getBorder() == juce::BorderSize<float>{ 0.f });
        }

        WHEN("the border-width has changed")
        {
            tree.setProperty("border-width", "9 8 7 6", nullptr);

            THEN("the box's border has changed")
            {
                REQUIRE(box.getBorder() == juce::BorderSize<float>{ 9.f, 6.f, 7.f, 8.f });
            }
        }
    }
}

//======================================================================================================================
SCENARIO("box models can have a margin")
{
    GIVEN("a box model")
    {
        juce::ValueTree tree{ "Component" };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree };
        jive::BoxModel box{ item, tree };

        THEN("the box has no margin")
        {
            REQUIRE(box.getMargin() == juce::BorderSize<float>{ 0.f });
        }

        WHEN("the margin has changed")
        {
            tree.setProperty("margin", "2 4 6 8", nullptr);

            THEN("the box's margin has changed")
            {
                REQUIRE(box.getMargin() == juce::BorderSize<float>{ 2.f, 8.f, 6.f, 4.f });
            }
        }
    }
}

//======================================================================================================================
SCENARIO("box models have a border bounds")
{
    GIVEN("a box model for a GUI item with a parent, with non-zero padding, border, and size properties")
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 25 },
              { "padding", "10 20" },
              { "border-width", "5" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        THEN("the box's border bounds has position (0, 0)")
        {
            REQUIRE(box.getBorderBounds().getPosition() == juce::Point<float>{ 0.f, 0.f });
        }
        AND_THEN("the border bounds' size takes into account the content size, the padding, and the border width")
        {
            REQUIRE(box.getBorderBounds().getWidth() == 150.f);
            REQUIRE(box.getBorderBounds().getHeight() == 55.f);
        }
    }
}

//======================================================================================================================
SCENARIO("box models have a padding bounds")
{
    GIVEN("a box model with non-zero padding, border, and size properties")
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 130 },
              { "height", 20 },
              { "padding", "15 5" },
              { "border-width", "10" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        THEN("the position of the box's padding bounds takes into account the border width")
        {
            REQUIRE(box.getPaddingBounds().getPosition() == juce::Point<float>{ 10.f, 10.f });
        }
        AND_THEN("the padding bounds' size takes into account the content size and the padding")
        {
            REQUIRE(box.getPaddingBounds().getWidth() == 140.f);
            REQUIRE(box.getPaddingBounds().getHeight() == 50.f);
        }
    }
}

//======================================================================================================================
SCENARIO("box models have a content bounds")
{
    GIVEN("a box model with non-zero padding, border, and size properties")
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 75 },
              { "height", 55 },
              { "padding", "30" },
              { "border-width", "5 10 15 20" } }
        };
        jive::GuiItem parentItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } };
        const jive::GuiItem item{ std::make_unique<juce::Component>(), tree, &parentItem };
        jive::BoxModel box{ item, tree };

        THEN("the position of the box's padding bounds takes into account the border width and the padding")
        {
            REQUIRE(box.getContentBounds().getPosition() == juce::Point<float>{ 50.f, 35.f });
        }
        AND_THEN("the content bounds' size matches the width and height properties of the box")
        {
            REQUIRE(box.getContentBounds().getWidth() == 75.f);
            REQUIRE(box.getContentBounds().getHeight() == 55.f);
        }
    }
}

//======================================================================================================================
SCENARIO("box models for top-level components don't expand their bounds")
{
    GIVEN("a GUI item with no parent, and its width, height, padding, and border-width properties specified")
    {
        juce::ValueTree tree{
            "Component",
            { { "width", 100 },
              { "height", 100 },
              { "padding", "25" },
              { "border-width", "5 10 15 20" } }
        };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        THEN("the size of the item's border bounds match its width and height")
        {
            REQUIRE(item.getBoxModel().getBorderBounds().getWidth() == 100.f);
            REQUIRE(item.getBoxModel().getBorderBounds().getHeight() == 100.f);
        }
    }
}
