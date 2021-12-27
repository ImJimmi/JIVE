#include <catch2/catch_test_macros.hpp>
#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
SCENARIO("GUI flex items can be laid-out in different orders")
{
    GIVEN("a GUI flex container with some children")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree1{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree1,
                                                                                          &item)));

        juce::ValueTree childTree2{ "Component", { { "width", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree2,
                                                                                          &item)));

        WHEN("the item's component resized such that it can contain all its children")
        {
            item.getComponent().setSize(150, 150);

            THEN("the children are laid-out in the same order they were added")
            {
                REQUIRE(item.getChild(0).getComponent().getRight() <= item.getChild(1).getComponent().getX());
            }

            WHEN("the children have their orders set to be in a different order")
            {
                childTree1.setProperty("order", 4, nullptr);
                childTree2.setProperty("order", 2, nullptr);

                THEN("the children are laid-out according to their new order")
                {
                    REQUIRE(item.getChild(0).getComponent().getX() >= item.getChild(1).getComponent().getRight());
                }

                WHEN("the children have their orders changed again")
                {
                    childTree1.setProperty("order", 7, nullptr);
                    childTree2.setProperty("order", 13, nullptr);

                    THEN("the children are laid-out according to their new order")
                    {
                        REQUIRE(item.getChild(0).getComponent().getRight() <= item.getChild(1).getComponent().getX());
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex items can grow to fill available space")
{
    GIVEN("a GUI flex container with some children, each with a flex-grow of 1")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree1{ "Component", { { "flex-grow", 1 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree1,
                                                                                          &item)));

        juce::ValueTree childTree2{ "Component", { { "flex-grow", 1 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree2,
                                                                                          &item)));

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

                THEN("the first child's component is twice the width of the second")
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
SCENARIO("GUI flex items can align themselves along their parent's cross-axis")
{
    GIVEN("a GUI flex container with a its children set to align at the end of its cross-axis, and a child")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
              { "align-items", juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree1{ "Component", { { "height", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree1,
                                                                                          &item)));

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
    GIVEN("a GUI flex container with a its children set to align at the start of its cross-axis, and a child")
    {
        juce::ValueTree tree{
            "Component",
            { { "flex-direction", juce::VariantConverter<juce::FlexBox::Direction>::toVar(juce::FlexBox::Direction::row) },
              { "align-items", juce::VariantConverter<juce::FlexBox::AlignItems>::toVar(juce::FlexBox::AlignItems::flexEnd) } },
            {}
        };
        jive::GuiFlexContainer item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        juce::ValueTree childTree1{ "Component", { { "height", 50 } } };
        item.addChild(std::make_unique<jive::GuiFlexItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                          childTree1,
                                                                                          &item)));

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

//======================================================================================================================
SCENARIO("GUI flex items should use the size of the item they decorate")
{
    GIVEN("a custom GUI item with a specific size")
    {
        struct StubGuiItem : public jive::GuiItem
        {
            StubGuiItem()
                : GuiItem{ std::make_unique<juce::Component>(), juce::ValueTree{ "Component" } }
            {
            }

            float getWidth() const override
            {
                return 123.f;
            }

            float getHeight() const override
            {
                return 987.f;
            }
        };

        auto item = std::make_unique<StubGuiItem>();

        WHEN("the item is decorated with a GUI flex item")
        {
            jive::GuiFlexItem flexItem{ std::move(item) };

            THEN("the flex item's width matches the width of the item it's decorating")
            {
                REQUIRE(flexItem.getWidth() == 123.f);
            }
            THEN("the flex item's height matches the height of the item it's decorating")
            {
                REQUIRE(flexItem.getHeight() == 987.f);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("GUI flex items can have a margin")
{
    GIVEN("a GUI flex item")
    {
        juce::ValueTree tree{ "Component" };
        jive::GuiFlexItem item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), tree) };

        WHEN("the item is converted to a juce::FlexItem")
        {
            const auto flexItem = static_cast<juce::FlexItem>(item);

            THEN("the flex-item has no margin")
            {
                REQUIRE(flexItem.margin.top == 0.0f);
                REQUIRE(flexItem.margin.right == 0.0f);
                REQUIRE(flexItem.margin.bottom == 0.0f);
                REQUIRE(flexItem.margin.left == 0.0f);
            }
        }
        WHEN("the item's margin changes")
        {
            tree.setProperty("margin", "1 2 3 4", nullptr);

            AND_WHEN("the item is converted to a juce::FlexItem")
            {
                const auto flexItem = static_cast<juce::FlexItem>(item);

                THEN("the flex-item's margin matches the one specified")
                {
                    REQUIRE(flexItem.margin.top == 1.0f);
                    REQUIRE(flexItem.margin.right == 2.0f);
                    REQUIRE(flexItem.margin.bottom == 3.0f);
                    REQUIRE(flexItem.margin.left == 4.0f);
                }
            }
        }
    }
}
