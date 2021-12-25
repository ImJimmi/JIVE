#include <catch2/catch_test_macros.hpp>
#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
SCENARIO("GUI flex containers can place items in different directions")
{
    GIVEN("a GUI flex container with some children")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          juce::ValueTree{ "Component" },
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          juce::ValueTree{ "Component" },
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          juce::ValueTree{ "Component" },
                                                                                          &item)));

        WHEN("the container's component is resized such that it is large enough to contain all of its children both "
             "side-by-side and one above the other")
        {
            item.getComponent().setSize(123, 456);

            THEN("the children are positioned vertically in the order they were added")
            {
                REQUIRE(item.getChild(0).getComponent().getBottom() == item.getChild(1).getComponent().getY());
                REQUIRE(item.getChild(1).getComponent().getBottom() == item.getChild(2).getComponent().getY());
            }

            AND_WHEN("the container's set to place items left-to-right")
            {
                tree.setProperty("flex-direction",
                                 juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row),
                                 nullptr);

                THEN("the children are positioned horizontally in the order they were added")
                {
                    REQUIRE(item.getChild(0).getComponent().getRight() == item.getChild(1).getComponent().getX());
                    REQUIRE(item.getChild(1).getComponent().getRight() == item.getChild(2).getComponent().getX());
                }
            }
            AND_WHEN("the container's set to place items bottom-to-top")
            {
                tree.setProperty("flex-direction",
                                 juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::columnReverse),
                                 nullptr);

                THEN("the children are positioned vertically with in the reverse order to how they were added")
                {
                    REQUIRE(item.getChild(0).getComponent().getY() == item.getChild(1).getComponent().getBottom());
                    REQUIRE(item.getChild(1).getComponent().getY() == item.getChild(2).getComponent().getBottom());
                }
            }
            AND_WHEN("the container's set to place items right-to-left")
            {
                tree.setProperty("flex-direction",
                                 juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::rowReverse),
                                 nullptr);

                THEN("the children are positioned horizontally in the reverse order to how they were added")
                {
                    REQUIRE(item.getChild(0).getComponent().getX() == item.getChild(1).getComponent().getRight());
                    REQUIRE(item.getChild(1).getComponent().getX() == item.getChild(2).getComponent().getRight());
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex containers can wrap items to multiple lines if needed")
{
    GIVEN("a GUI flex container with some children")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree{ "Component", { { "width", 120 }, { "height", 100 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));

        WHEN("the container's component is resized such that it's not wide enough to contain all of the children at "
             "their desired widths")
        {
            item.getComponent().setSize(300, 400);

            THEN("the children are squashed to fit side-by-side")
            {
                REQUIRE(item.getChild(0).getComponent().getWidth() == 100);
                REQUIRE(item.getChild(1).getComponent().getWidth() == 100);
                REQUIRE(item.getChild(2).getComponent().getWidth() == 100);
            }

            AND_WHEN("the container's set to wrap its items")
            {
                tree.setProperty("flex-wrap",
                                 juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrap),
                                 nullptr);

                THEN("the children have their desired widths and any that wouldn't otherwise have fit are wrapped")
                {
                    REQUIRE(item.getChild(0).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(1).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(2).getComponent().getWidth() == 120);

                    REQUIRE(item.getChild(0).getComponent().getY() == 0);
                    REQUIRE(item.getChild(1).getComponent().getY() == 0);
                    REQUIRE(item.getChild(2).getComponent().getY() == 200);
                }
            }
            AND_WHEN("the container's set to wrap it's items in reverse")
            {
                tree.setProperty("flex-wrap",
                                 juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrapReverse),
                                 nullptr);

                THEN("the children have their designed widths and any that wouldn't otherwise have fit are wrapped in "
                     "reverse")
                {
                    REQUIRE(item.getChild(0).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(1).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(2).getComponent().getWidth() == 120);

                    REQUIRE(item.getChild(0).getComponent().getY() == 300);
                    REQUIRE(item.getChild(1).getComponent().getY() == 300);
                    REQUIRE(item.getChild(2).getComponent().getY() == 100);
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex containers can justify their content along their main axis")
{
    GIVEN("a GUI flex container with some children")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree{ "Component", { { "width", 10 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));

        WHEN("the container's component is resized such that it was wider than the total width of all its children")
        {
            item.getComponent().setSize(90, 100);

            THEN("the children are placed on the left")
            {
                REQUIRE(item.getChild(0).getComponent().getX() == 0);
                REQUIRE(item.getChild(1).getComponent().getX() == item.getChild(0).getComponent().getRight());
                REQUIRE(item.getChild(2).getComponent().getX() == item.getChild(1).getComponent().getRight());
            }

            AND_WHEN("the container's set to justify its children to the end of its main axis")
            {
                tree.setProperty("justify-content",
                                 juce::VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent::flexEnd),
                                 nullptr);

                THEN("the children are placed at the end of its main axis")
                {
                    REQUIRE(item.getChild(2).getComponent().getRight() == 90);
                    REQUIRE(item.getChild(1).getComponent().getRight() == item.getChild(2).getComponent().getX());
                    REQUIRE(item.getChild(0).getComponent().getRight() == item.getChild(1).getComponent().getX());
                }
            }
            AND_WHEN("the container's set to justify its children in the centre of its main axis")
            {
                tree.setProperty("justify-content",
                                 juce::VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent::center),
                                 nullptr);

                THEN("the children are placed in the centre")
                {
                    REQUIRE(item.getChild(0).getComponent().getBounds().getCentreX() == 35);
                    REQUIRE(item.getChild(1).getComponent().getBounds().getCentreX() == 45);
                    REQUIRE(item.getChild(2).getComponent().getBounds().getCentreX() == 55);
                }
            }
            AND_WHEN("the container's set to justify its children to have the maximum amount of space between them")
            {
                tree.setProperty("justify-content",
                                 juce::VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent::spaceBetween),
                                 nullptr);

                THEN("the children are spaced with the maximum amount of space between them")
                {
                    REQUIRE(item.getChild(0).getComponent().getX() == 0);
                    REQUIRE(item.getChild(1).getComponent().getBounds().getCentreX() == 45);
                    REQUIRE(item.getChild(2).getComponent().getRight() == 90);
                }
            }
            AND_WHEN("the container's set to justify its children to have the same amount of space around them")
            {
                tree.setProperty("justify-content",
                                 juce::VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent::spaceAround),
                                 nullptr);

                THEN("the children are spaced with an equal space on either side of them")
                {
                    REQUIRE(item.getChild(0).getComponent().getX() == 10);
                    REQUIRE(item.getChild(1).getComponent().getBounds().getCentreX() == 45);
                    REQUIRE(item.getChild(2).getComponent().getRight() == 80);
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex containers can align their items along their cross-axis")
{
    GIVEN("a GUI flex container with a child")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));

        WHEN("the container's component is resized")
        {
            item.getComponent().setSize(100, 150);

            THEN("the child is stretched to cover the cross-axis of the parent")
            {
                REQUIRE(item.getChild(0).getComponent().getY() == 0);
                REQUIRE(item.getChild(0).getComponent().getHeight() == 150);
            }

            AND_WHEN("the child is given a fixed height")
            {
                childTree.setProperty("height", 50, nullptr);

                AND_WHEN("the container's set to align its items at the start of its cross-axis")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexStart),
                                     nullptr);

                    THEN("the child is aligned at the start of the parent's cross-axis")
                    {
                        REQUIRE(item.getChild(0).getComponent().getY() == 0);
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                    }
                }
                AND_WHEN("the container's set to align its items at the end of its cross-axis")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd),
                                     nullptr);

                    THEN("the child is aligned at the end of the parent's cross-axis")
                    {
                        REQUIRE(item.getChild(0).getComponent().getBottom() == 150);
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                    }
                }
                AND_WHEN("the container's set to align its items at the centre of its cross-axis")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::center),
                                     nullptr);

                    THEN("the child is aligned at the centre of the parent's cross-axis")
                    {
                        REQUIRE(item.getChild(0).getComponent().getBounds().getCentreY() == 75);
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex containers can align lines of their items along their cross-axis")
{
    GIVEN("a GUI flex container with wrapping enabled, and some children")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
              { "flex-wrap", juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrap) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));

        WHEN("the container's component is resized such that it isn't wide enough to contain both its children, but "
             "more than tall enough to contain them")
        {
            item.getComponent().setSize(90, 200);

            THEN("the lines of children are aligned at the start of the parent's cross-axis")
            {
                REQUIRE(item.getChild(0).getComponent().getY() == 0);
                REQUIRE(item.getChild(1).getComponent().getY() == 100);
            }

            AND_WHEN("the container's set to align lines of its items at the end of its cross-axis, and the children "
                     "are given a fixed height")
            {
                childTree.setProperty("height", 50, nullptr);
                tree.setProperty("align-content",
                                 juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::flexEnd),
                                 nullptr);

                THEN("the lines of children are aligned at the end of the parent's cross-axis")
                {
                    REQUIRE(item.getChild(0).getComponent().getBottom() == 150);
                    REQUIRE(item.getChild(1).getComponent().getBottom() == 200);
                }

                AND_WHEN("the container's set to align lines of its items at the centre of its cross-axis")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::center),
                                     nullptr);

                    THEN("the lines of children are aligned at the centre of the parent's cross-axis")
                    {
                        REQUIRE(item.getChild(0).getComponent().getBottom() == 100);
                        REQUIRE(item.getChild(1).getComponent().getY() == 100);
                    }
                }
                AND_WHEN("the container's set to align lines of its children to have the maximum amount of space "
                         "between them")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::spaceBetween),
                                     nullptr);

                    THEN("the lines of children are aligned to have the maximum amount of space between them")
                    {
                        REQUIRE(item.getChild(0).getComponent().getY() == 0);
                        REQUIRE(item.getChild(1).getComponent().getBottom() == 200);
                    }
                }
                AND_WHEN("the container's set to align lines of its children to have the same amount of space around "
                         "them")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::spaceAround),
                                     nullptr);

                    THEN("the lines of children are aligned with the same amount of space around them")
                    {
                        REQUIRE(item.getChild(0).getComponent().getY() == 25);
                        REQUIRE(item.getChild(1).getComponent().getBottom() == 175);
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex items implement padding")
{
    GIVEN("a GUI flex item with a child item")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree{ "Component" };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree,
                                                                                          &item)));

        WHEN("the item's component is resized")
        {
            item.getComponent().setSize(100, 100);

            THEN("the first child of the item's component has no padding applied")
            {
                const auto childBounds = item.getComponent().getChildComponent(0)->getBounds();

                REQUIRE(childBounds.getX() == 0);
                REQUIRE(childBounds.getY() == 0);
                REQUIRE(childBounds.getWidth() == 100);
            }
        }

        WHEN("the item's padding is changed")
        {
            tree.setProperty("padding", "10 20 30 40", nullptr);

            AND_WHEN("the item's component is resized")
            {
                item.getComponent().setSize(220, 330);

                THEN("the first child of the item's component has the padding applied")
                {
                    const auto childBounds = item.getComponent().getChildComponent(0)->getBounds();

                    REQUIRE(childBounds.getX() == 40);
                    REQUIRE(childBounds.getY() == 10);
                    REQUIRE(childBounds.getWidth() == 160);
                }
            }
        }
    }
}
