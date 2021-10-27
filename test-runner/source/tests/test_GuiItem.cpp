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
SCENARIO("GUI items have a flex-direction property")
{
    GIVEN("a GUI item with a flex layout and some children")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component" }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component" }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component" }));

        WHEN("the item's component is resized such that it is large enough to contain all of its children both "
             "side-by-side and one above the other")
        {
            item.getComponent().setSize(123, 456);

            THEN("the children are positioned vertically in the order they were added")
            {
                REQUIRE(item.getChild(0).getComponent().getBottom() == item.getChild(1).getComponent().getY());
                REQUIRE(item.getChild(1).getComponent().getBottom() == item.getChild(2).getComponent().getY());
            }

            WHEN("the item has a row flex-direction")
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
            WHEN("the item has a column-reverse flex-direction")
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
            WHEN("the item has a row-reverse flex-direction")
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
SCENARIO("GUI items have a flex-wrap property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, and some children")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 120 } } }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 120 } } }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 120 } } }));

        WHEN("the item's component is resized such that it's not wide enough to contain all of the children at their "
              "desired widths")
        {
            item.getComponent().setSize(300, 400);

            THEN("the children are squashed to fit side-by-side")
            {
                REQUIRE(item.getChild(0).getComponent().getWidth() == 100);
                REQUIRE(item.getChild(1).getComponent().getWidth() == 100);
                REQUIRE(item.getChild(2).getComponent().getWidth() == 100);
            }

            WHEN("the item has wrapping enabled")
            {
                tree.setProperty("flex-wrap",
                                 juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrap),
                                 nullptr);

                THEN("the children have their desired widths and any that wouldn't otherwise have fit are dropped to "
                     "the next row")
                {
                    REQUIRE(item.getChild(0).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(1).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(2).getComponent().getWidth() == 120);

                    REQUIRE(item.getChild(0).getComponent().getY() == 0);
                    REQUIRE(item.getChild(1).getComponent().getY() == 0);
                    REQUIRE(item.getChild(2).getComponent().getY() == 200);
                }
            }
            WHEN("the item has wrap-reverse enabled")
            {
                tree.setProperty("flex-wrap",
                                 juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrapReverse),
                                 nullptr);

                THEN("the children have their designed widths and any that wouldn't otherwise have fit are raised to the "
                     "row above")
                {
                    REQUIRE(item.getChild(0).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(1).getComponent().getWidth() == 120);
                    REQUIRE(item.getChild(2).getComponent().getWidth() == 120);

                    REQUIRE(item.getChild(0).getComponent().getY() == 200);
                    REQUIRE(item.getChild(1).getComponent().getY() == 200);
                    REQUIRE(item.getChild(2).getComponent().getY() == 0);
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items have a justify-content property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, and some children")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 10 } } }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 10 } } }));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(),
                                                      juce::ValueTree{ "Component", { { "width", 10 } } }));

        WHEN("the item is resized such that it was wider than the total width of all its children")
        {
            item.getComponent().setSize(90, 100);

            THEN("the children are placed on the left")
            {
                REQUIRE(item.getChild(0).getComponent().getX() == 0);
                REQUIRE(item.getChild(1).getComponent().getX() == item.getChild(0).getComponent().getRight());
                REQUIRE(item.getChild(2).getComponent().getX() == item.getChild(1).getComponent().getRight());
            }

            WHEN("the item has a flex-end justify-content")
            {
                tree.setProperty("justify-content",
                                 juce::VariantConverter<juce::FlexBox::JustifyContent>::toVar(juce::FlexBox::JustifyContent::flexEnd),
                                 nullptr);

                THEN("the children are placed on the right")
                {
                    REQUIRE(item.getChild(2).getComponent().getRight() == 90);
                    REQUIRE(item.getChild(1).getComponent().getRight() == item.getChild(2).getComponent().getX());
                    REQUIRE(item.getChild(0).getComponent().getRight() == item.getChild(1).getComponent().getX());
                }
            }
            WHEN("the item has a centre justify-content")
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
            WHEN("the item has a space-between justify-content")
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
            WHEN("the item has a space-between justify-content")
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
SCENARIO("GUI items have an align-items property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, and a child")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree));

        WHEN("the item is resized")
        {
            item.getComponent().setSize(100, 150);

            THEN("the height of the child's component matches the height of the parent's component")
            {
                REQUIRE(item.getChild(0).getComponent().getHeight() == 150);
            }

            WHEN("the child is given a fixed height")
            {
                childTree.setProperty("height", 50, nullptr);

                WHEN("the item has a flex-start item alignment")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexStart),
                                     nullptr);

                    THEN("the child is aligned at the top of the parent")
                    {
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                        REQUIRE(item.getChild(0).getComponent().getY() == 0);
                    }
                }
                WHEN("the item has a flex-end item alignment")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd),
                                     nullptr);

                    THEN("the child is aligned at the bottom of the parent")
                    {
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                        REQUIRE(item.getChild(0).getComponent().getBottom() == 150);
                    }
                }
                WHEN("the item has a flex-end item alignment")
                {
                    tree.setProperty("align-items",
                                     juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::center),
                                     nullptr);

                    THEN("the child is aligned in the centre of the parent")
                    {
                        REQUIRE(item.getChild(0).getComponent().getHeight() == 50);
                        REQUIRE(item.getChild(0).getComponent().getBounds().getCentreY() == 75);
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items have an align-content property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, a wrap flex-wrap, and two children")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
                { "flex-wrap", juce::VariantConverter<juce::FlexBox::Wrap>::toVar(juce::FlexBox::Wrap::wrap) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree));
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree));

        WHEN("the item's component is resized such that it isn't wide enough to contain both its children, but more "
             "than tall enough to contain them")
        {
            item.getComponent().setSize(90, 200);

            THEN("the children are aligned at the top of the parent")
            {
                REQUIRE(item.getChild(0).getComponent().getY() == 0);
                REQUIRE(item.getChild(1).getComponent().getY() == 100);
            }

            WHEN("the item's align-content property is set to 'flex-end', and the children are given a fixed height")
            {
                childTree.setProperty("height", 50, nullptr);
                tree.setProperty("align-content",
                                 juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::flexEnd),
                                 nullptr);

                THEN("the children are aligned at the bottom of the parent")
                {
                    REQUIRE(item.getChild(0).getComponent().getBottom() == 150);
                    REQUIRE(item.getChild(1).getComponent().getBottom() == 200);
                }

                WHEN("the item's align-content property is set to 'centre'")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::center),
                                     nullptr);

                    THEN("the children are aligned in the centre of the parent")
                    {
                        REQUIRE(item.getChild(0).getComponent().getBottom() == 100);
                        REQUIRE(item.getChild(1).getComponent().getY() == 100);
                    }
                }
                WHEN("the item's align-content property is set to 'space-between'")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::spaceBetween),
                                     nullptr);

                    THEN("the children are aligned with the maximum space between them")
                    {
                        REQUIRE(item.getChild(0).getComponent().getY() == 0);
                        REQUIRE(item.getChild(1).getComponent().getBottom() == 200);
                    }
                }
                WHEN("the item's align-content property is set to 'space-around'")
                {
                    tree.setProperty("align-content",
                                     juce::VariantConverter<juce::FlexBox::AlignContent>::toVar(juce::FlexBox::AlignContent::spaceAround),
                                     nullptr);

                    THEN("the children are aligned with the same amount of space around them")
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
SCENARIO("GUI items have an 'order' property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, and two children")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree1{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree1, &item));

        juce::ValueTree childTree2{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree2, &item));

        WHEN("the item's component resized such that it can contain both children side-by-side")
        {
            item.getComponent().setSize(150, 150);

            THEN("the first child's component is the the left of the second")
            {
                REQUIRE(item.getChild(0).getComponent().getRight() <= item.getChild(1).getComponent().getX());
            }

            WHEN("the first item's order is changed to be larger than the second's order")
            {
                childTree1.setProperty("order", 4, nullptr);

                THEN("the first child's component is to the right of the second")
                {
                    REQUIRE(item.getChild(0).getComponent().getX() >= item.getChild(1).getComponent().getRight());
                }

                WHEN("the second item;s order is changed to be large than the first's order")
                {
                    childTree2.setProperty("order", 7, nullptr);

                    THEN("the first child's component is to the left of the second")
                    {
                        REQUIRE(item.getChild(0).getComponent().getRight() <= item.getChild(1).getComponent().getX());
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items have a 'flex-grow' property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, and two children with a flex-grow of 1")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree1{ "Component", { { "flex-grow", 1 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree1, &item));

        juce::ValueTree childTree2{ "Component", { { "flex-grow", 1 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree2, &item));

        WHEN("the item's component is resized")
        {
            item.getComponent().setSize(300, 100);

            THEN("the children have the same width")
            {
                REQUIRE(item.getChild(0).getComponent().getWidth() == item.getChild(1).getComponent().getWidth());
            }

            WHEN("the first child's flex-grow is set to double that of the second")
            {
                childTree1.setProperty("flex-grow", 2, nullptr);

                THEN("the first child's component is twice the width of the first")
                {
                    REQUIRE(item.getChild(0).getComponent().getWidth() == item.getChild(1).getComponent().getWidth() * 2);
                }

                WHEN("the second child's flex-grow is set to 1.5x that of the first")
                {
                    childTree2.setProperty("flex-grow", 3, nullptr);

                    THEN("the second child'c component is 1.5x the width of the second")
                    {
                        REQUIRE(item.getChild(1).getComponent().getWidth() == item.getChild(0).getComponent().getWidth() * 1.5);
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI items have an align-self property")
{
    GIVEN("a GUI item with a flex layout, a row flex-direction, a flex-end item alignment, and a child")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
                { "align-items", juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree1{ "Component", { { "height", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree1, &item));

        WHEN("the item's component is resized such that it's more than tall enough to contain its child")
        {
            item.getComponent().setSize(100, 200);

            THEN("the child's component is aligned at the bottom of the parent")
            {
                REQUIRE(item.getChild(0).getComponent().getY() == 150);
            }

            WHEN("the child's align-self is set to 'flex-start'")
            {
                childTree1.setProperty("align-self",
                                       juce::VariantConverter<juce::FlexItem::AlignSelf>::toVar(juce::FlexItem::AlignSelf::flexStart),
                                       nullptr);

                THEN("the child's component is aligned at the top of the parent")
                {
                    REQUIRE(item.getChild(0).getComponent().getY() == 0);
                }
            }
            WHEN("the child's align-self is set to 'centre'")
            {
                childTree1.setProperty("align-self",
                                       juce::VariantConverter<juce::FlexItem::AlignSelf>::toVar(juce::FlexItem::AlignSelf::center),
                                       nullptr);

                THEN("the child's component is aligned in the centre of the parent")
                {
                    REQUIRE(item.getChild(0).getComponent().getBounds().getCentreY() == 100);
                }
            }
            WHEN("the child's align-self is set to 'stretch'")
            {
                childTree1.setProperty("align-self",
                                       juce::VariantConverter<juce::FlexItem::AlignSelf>::toVar(juce::FlexItem::AlignSelf::stretch),
                                       nullptr);

                THEN("the child's component is aligned at the top of the parent")
                {
                    REQUIRE(item.getChild(0).getComponent().getY() == 0);
                }
            }
        }
    }
    GIVEN("a GUI item with a flex layout, a row flex-direction, a flex-start item alignment, and a child")
    {
        juce::ValueTree tree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
                { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
                { "align-items", juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd) }
            },
            {}
        };
        jive::GuiItem item{ test::createDummyComponent(), tree };

        juce::ValueTree childTree1{ "Component", { { "height", 50 } } };
        item.addChild(std::make_unique<jive::GuiItem>(test::createDummyComponent(), childTree1, &item));

        WHEN("the item's component is resized such that it's more than tall enough to contain its child")
        {
            item.getComponent().setSize(100, 200);

            WHEN("the child's align-self is set to 'flex-end'")
            {
                childTree1.setProperty("align-self",
                                       juce::VariantConverter<juce::FlexItem::AlignSelf>::toVar(juce::FlexItem::AlignSelf::flexEnd),
                                       nullptr);

                THEN("the child's component is aligned at the bottom of the parent")
                {
                    REQUIRE(item.getChild(0).getComponent().getBottom() == item.getComponent().getBottom());
                }
            }
        }
    }
}
