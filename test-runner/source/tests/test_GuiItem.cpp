#include <utilities/jive_UnitTest.h>

#include <jive/jive.h>

//======================================================================================================================
class GuiItemTests : public jive::UnitTest
{
public:
    //==================================================================================================================
    GuiItemTests()
        : jive::UnitTest{ "jive::GuiItem" }
    {
    }

    //==================================================================================================================
    void runTest() final
    {
        testChildren();
        testComponentIDs();
        testDisplayProperty();

        testFlexDirection();
        testFlexWrap();
        testFlexJustifyContent();
        testFlexAlignContent();
        testFlexItemOrder();
        testFlexItemGrow();
        testFlexItemSize();
    }

private:
    //==================================================================================================================
    void testChildren()
    {
        beginTest("Test children");

        GIVEN("a GUI item created from a value tree with no children");
        jive::GuiItem item{ std::make_unique<juce::Component>(), juce::ValueTree{ "Dummy" } };

        {
            THEN("the item should have no children");
            expectEquals(item.getNumChildren(), 0);

            THEN("the item's component should have no children");
            expectEquals(item.getComponent().getNumChildComponents(), 0);
        }

        {
            WHEN("a child is added to the item");
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "AlsoADummy" }));

            THEN("the item should have a single child");
            expectEquals(item.getNumChildren(), 1);

            THEN("the item's component should have a single child");
            expectEquals(item.getComponent().getNumChildComponents(), 1);
        }

        {
            WHEN("a second child is added to the item");
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "YetAnotherDummy" }));

            THEN("the item should have 2 children");
            expectEquals(item.getNumChildren(), 2);

            THEN("the item's component should have 2 children");
            expectEquals(item.getComponent().getNumChildComponents(), 2);
        }
    }

    void testComponentIDs()
    {
        beginTest("Test Component IDs");

        GIVEN("a GUI item");
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        {
            THEN("the item's ID is blank");
            expectEquals(item.getComponent().getComponentID(), juce::String{});
        }

        {
            WHEN("the value-tree being used by the item has its 'id' property set to '123'");
            tree.setProperty("id", "123", nullptr);

            THEN("the item's ID is '123'.");
            expectEquals(item.getComponent().getComponentID(), juce::String{ "123" });
        }
        {
            WHEN("the value-tree begin used by the item has its 'id' property set to '456'");
            tree.setProperty("id", "456", nullptr);

            THEN("the item's ID is '456'.");
            expectEquals(item.getComponent().getComponentID(), juce::String{ "456" });
        }
    }

    void testDisplayProperty()
    {
        beginTest("Test display types");

        GIVEN("a GUI item");
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        {
            WHEN("the value-tree being used by the item has its 'display' property set to 'flex'");
            tree.setProperty("display", "flex", nullptr);

            THEN("the item's display type is 'flex'");
            expect(item.getDisplay() == jive::GuiItem::Display::flex);
        }
        {
            WHEN("the value-tree being used by the item has its 'display' property set to 'grid'");
            tree.setProperty("display", "grid", nullptr);

            THEN("the item's display type is 'grid'");
            expect(item.getDisplay() == jive::GuiItem::Display::grid);
        }
    }

    void testFlexDirection()
    {
        beginTest("Test flex direction");

        {
            GIVEN("a GUI item with 2 children, a flex layout, and its 'flex-direction' property set to 'column'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "column", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));

            {
                WHEN("the parent item's component is given a size of [50, 75]");
                item.getComponent().setSize(50, 75);

                THEN("the width of the first child's component should match the width of the parent component");
                expectEquals(item.getChild(0).getComponent().getWidth(), 50);

                THEN("the width of the second child's component should match the width of the parent component");
                expectEquals(item.getChild(1).getComponent().getWidth(), 50);

                THEN("the second child's component is positioned below the first child's component");
                expectGreaterOrEqual(item.getChild(1).getComponent().getY(),
                                     item.getChild(0).getComponent().getBottom());
            }
        }
        {
            GIVEN("a GUI item with 2 children, a flex layout, and its 'flex-direction' property set to 'row'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));

            {
                WHEN("the parent item's component is given a size of [80, 120]");
                item.getComponent().setSize(80, 120);

                THEN("the height of the first child's component should match the height of the parent component");
                expectEquals(item.getChild(0).getComponent().getHeight(), 120);

                THEN("the height of the second child's component should match the height of the parent component");
                expectEquals(item.getChild(1).getComponent().getHeight(), 120);

                THEN("the second child's component is positioned to the right of the first child's component");
                expectGreaterOrEqual(item.getChild(1).getComponent().getX(),
                                     item.getChild(0).getComponent().getRight());
            }
        }
        {
            GIVEN("a GUI item with 2 children, a flex layout, and its 'flex-direction' property set to "
                  "'column-reverse'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "column-reverse", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));

            {
                WHEN("the parent item's component is given a size of [20, 40]");
                item.getComponent().setSize(20, 40);

                THEN("the width of the first child's component should match the width of the parent component");
                expectEquals(item.getChild(0).getComponent().getWidth(), 20);

                THEN("the width of the second child's component should match the width of the parent component");
                expectEquals(item.getChild(1).getComponent().getWidth(), 20);

                THEN("the second child's component is positioned above the first child's component");
                expectLessOrEqual(item.getChild(1).getComponent().getBottom(),
                                  item.getChild(0).getComponent().getY());
            }
        }
        {
            GIVEN("a GUI item with 2 children, a flex layout, and its 'flex-direction' property set to 'row-reverse'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "row-reverse", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" }));

            {
                WHEN("the parent item's component is given a size of [20, 40]");
                item.getComponent().setSize(150, 200);

                THEN("the height of the first child's component should match the height of the parent component");
                expectEquals(item.getChild(0).getComponent().getHeight(), 200);

                THEN("the height of the second child's component should match the height of the parent component");
                expectEquals(item.getChild(1).getComponent().getHeight(), 200);

                THEN("the second child's component is positioned to the left of the first child's component");
                expectLessOrEqual(item.getChild(1).getComponent().getRight(),
                                  item.getChild(0).getComponent().getX());
            }
        }
    }

    void testFlexWrap()
    {
        beginTest("Test flex wrap");

        {
            GIVEN("a GUI item with 3 children each with a width of 30, a flex layout, and its 'flex-wrap' property set "
                  "to 'no-wrap' and its 'flex-direction' property set to 'row'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-wrap", "nowrap", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Component" };
            childTree.setProperty("width", 30, nullptr);
            childTree.setProperty("height", 10, nullptr);

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));

            {
                WHEN("the parent item's component is given a size of [80, 80]");
                item.getComponent().setSize(80, 80);

                THEN("all three children have the same Y position");
                expectEquals(item.getChild(0).getComponent().getY(), item.getChild(1).getComponent().getY());
                expectEquals(item.getChild(1).getComponent().getY(), item.getChild(2).getComponent().getY());
            }
        }
        {
            GIVEN("a GUI item with 3 children each with a width of 30, a flex layout, and its 'flex-wrap' property set "
                  "to 'no-wrap'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-wrap", "wrap", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Component" };
            childTree.setProperty("width", 30, nullptr);
            childTree.setProperty("height", 10, nullptr);

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));

            {
                WHEN("the parent item's component is given a size of [80, 80]");
                item.getComponent().setSize(80, 80);

                THEN("the first two children's components have the same Y position");
                expectEquals(item.getChild(0).getComponent().getY(), item.getChild(1).getComponent().getY());

                THEN("the last child's component is positioned below the first two");
                expectGreaterOrEqual(item.getChild(2).getComponent().getY(), item.getChild(0).getComponent().getBottom());
            }
        }
        {
            GIVEN("a GUI item with 3 children each with a width of 30, a flex layout, and its 'flex-wrap' property set "
                  "to 'no-wrap'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-wrap", "wrap-reverse", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);

            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Component" };
            childTree.setProperty("width", 30, nullptr);
            childTree.setProperty("height", 10, nullptr);

            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));

            {
                WHEN("the parent item's component is given a size of [80, 80]");
                item.getComponent().setSize(80, 80);

                THEN("the first two children's components have the same Y position");
                expectEquals(item.getChild(0).getComponent().getY(), item.getChild(1).getComponent().getY());

                THEN("the last child's component is positioned above the first two");
                expectGreaterOrEqual(item.getChild(0).getComponent().getY(), item.getChild(2).getComponent().getBottom());
            }
        }
    }

    void testFlexJustifyContent()
    {
        beginTest("Test flex justify-content");

        {
            GIVEN("a GUI item with two children each with a height of 50, and a flex layout");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Child" };
            childTree.setProperty("height", 50, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));

            {
                WHEN("the parent item's 'justify-content' property is set to 'flex-start', and its component is given "
                     "a size of [200, 200]");
                tree.setProperty("justify-content", "flex-start", nullptr);
                item.getComponent().setSize(200, 200);

                THEN("the Y position of the first child's component is 0");
                expectEquals(item.getChild(0).getComponent().getY(), 0);

                THEN("the Y position of the second child's component is equal to the Y position of the bottom edge of "
                     "the first child's component");
                expectEquals(item.getChild(1).getComponent().getY(), item.getChild(0).getComponent().getBottom());
            }
            {
                WHEN("the parent item's 'justify-content' property is set to 'flex-end'");
                tree.setProperty("justify-content", "flex-end", nullptr);

                THEN("the Y position of the bottom edge of the second child's component is 150");
                expectEquals(item.getChild(1).getComponent().getBottom(), 200);

                THEN("the Y position of the bottom edge of the first child's component is equal to the Y position of "
                     "the second child");
                expectEquals(item.getChild(0).getComponent().getBottom(), item.getChild(1).getComponent().getY());
            }
            {
                WHEN("the parent item's 'justify-content' property is set to 'centre'");
                tree.setProperty("justify-content", "centre", nullptr);

                THEN("the Y position of the bottom edge of the first child's component is 75");
                expectEquals(item.getChild(0).getComponent().getBottom(), 100);

                THEN("the Y position of the second child's component is 75");
                expectEquals(item.getChild(1).getComponent().getY(), 100);
            }
            {
                WHEN("the parent item's 'justify-content' property is set to 'space-between'");
                tree.setProperty("justify-content", "space-between", nullptr);

                THEN("the Y position of the first child's component is 0");
                expectEquals(item.getChild(0).getComponent().getY(), 0);

                THEN("the Y position of the bottom edge of the second child's component is 150");
                expectEquals(item.getChild(1).getComponent().getBottom(), 200);
            }
            {
                WHEN("the parent item's 'justify-content' property is set to 'space-around'");
                tree.setProperty("justify-content", "space-around", nullptr);

                THEN("the Y position of the first child's component is 25");
                expectEquals(item.getChild(0).getComponent().getY(), 25);

                THEN("the Y position of the first child's component is 125");
                expectEquals(item.getChild(1).getComponent().getY(), 125);
            }
        }
    }

    void testFlexAlignContent()
    {
        beginTest("Test flex align-content");

        {
            GIVEN("a GUI item with two children each with a size of 50x50, a flex layout, and its 'flex-wrap' property "
                  "set to 'wrap'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-wrap", "wrap", nullptr);
            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Child" };
            childTree.setProperty("width", 50, nullptr);
            childTree.setProperty("height", 50, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree));

            {
                WHEN("the parent's 'align-content' property is set to 'stretch', and its component is given a size of "
                     "[200, 90]");
                tree.setProperty("align-content", "stretch", nullptr);
                item.getComponent().setSize(200, 90);

                THEN("the X position of the first child's component is 0");
                expectEquals(item.getChild(0).getComponent().getX(), 0);

                THEN("the X position of the second child's component is 100");
                expectEquals(item.getChild(1).getComponent().getX(), 100);
            }
            {
                WHEN("the parent's 'align-content' property is set to 'flex-start'");
                tree.setProperty("align-content", "flex-start", nullptr);

                THEN("the X position of the first child's component is 0");
                expectEquals(item.getChild(0).getComponent().getX(), 0);

                THEN("the X position of the second child's component is 50");
                expectEquals(item.getChild(1).getComponent().getX(), 50);
            }
            {
                WHEN("the parent's 'align-content' property is set to 'flex-end'");
                tree.setProperty("align-content", "flex-end", nullptr);

                THEN("the X position of the first child's component is 100");
                expectEquals(item.getChild(0).getComponent().getX(), 100);

                THEN("the X position of the second child's component is 150");
                expectEquals(item.getChild(1).getComponent().getX(), 150);
            }
            {
                WHEN("the parent's 'align-content' property is set to 'centre'");
                tree.setProperty("align-content", "centre", nullptr);

                THEN("the X position of the first child's component is 50");
                expectEquals(item.getChild(0).getComponent().getX(), 50);

                THEN("the X position of the second child's component is 100");
                expectEquals(item.getChild(1).getComponent().getX(), 100);
            }
            {
                WHEN("the parent's 'align-content' property is set to 'space-between'");
                tree.setProperty("align-content", "space-between", nullptr);

                THEN("the X position of the first child's component is 0");
                expectEquals(item.getChild(0).getComponent().getX(), 0);

                THEN("the X position of the second child's component is 150");
                expectEquals(item.getChild(1).getComponent().getX(), 150);
            }
            {
                WHEN("the parent's 'align-content' property is set to 'space-around'");
                tree.setProperty("align-content", "space-around", nullptr);

                THEN("the X position of the first child's component is 25");
                expectEquals(item.getChild(0).getComponent().getX(), 25);

                THEN("the X position of the second child's component is 125");
                expectEquals(item.getChild(1).getComponent().getX(), 125);
            }
        }
    }

    void testFlexItemOrder()
    {
        beginTest("Test flex-item order");

        {
            GIVEN("a GUI item with 2 children, a flex layout, and a flex-direction of 'row'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);
            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree1{ "Component" };
            childTree1.setProperty("width", 50, nullptr);
            childTree1.setProperty("height", 50, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree1, &item));

            juce::ValueTree childTree2{ "Component" };
            childTree2.setProperty("width", 50, nullptr);
            childTree2.setProperty("height", 50, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree2, &item));

            {
                WHEN("the first child is given an order of 1, the second child an order of 2, and the item is resized");
                childTree1.setProperty("order", 1, nullptr);
                childTree2.setProperty("order", 2, nullptr);
                item.getComponent().setSize(200, 200);

                THEN("the first child's component is positioned on the left");
                expect(item.getChild(0).getComponent().getBounds() == juce::Rectangle<int>{ 0, 0, 50, 50 });

                THEN("the second child's component is position to the right of the first");
                expect(item.getChild(1).getComponent().getBounds() == juce::Rectangle<int>{ 50, 0, 50, 50 });
            }
            {
                WHEN("the first child is given an order of 7, and the second child an order of 4");
                childTree1.setProperty("order", 7, nullptr);
                childTree2.setProperty("order", 4, nullptr);

                THEN("the first child's component is positioned to the right of the second");
                expect(item.getChild(0).getComponent().getBounds() == juce::Rectangle<int>{ 50, 0, 50, 50 });

                THEN("the second child's component is position on the left");
                expect(item.getChild(1).getComponent().getBounds() == juce::Rectangle<int>{ 0, 0, 50, 50 });
            }
        }
    }

    void testFlexItemGrow()
    {
        beginTest("Test flex-item grow");

        {
            GIVEN("a GUI item with 2 children each with a fixed height and a flex-grow of 1, a flex layout, and a flex-direction of 'row'");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            tree.setProperty("flex-direction", "row", nullptr);
            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree1{ "Component" };
            childTree1.setProperty("height", 50, nullptr);
            childTree1.setProperty("flex-grow", 1, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree1, &item));

            juce::ValueTree childTree2{ "Component" };
            childTree2.setProperty("height", 50, nullptr);
            childTree2.setProperty("flex-grow", 1, nullptr);
            item.addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree2));

            {
                WHEN("the item is resized");
                item.getComponent().setSize(300, 100);

                THEN("the first child's component covers the left half of the parent's width");
                expect(item.getChild(0).getComponent().getBounds() == juce::Rectangle<int>{ 0, 0, 150, 50 });

                THEN("the second child's component covers the right half of the parent's width");
                expect(item.getChild(1).getComponent().getBounds() == juce::Rectangle<int>{ 150, 0, 150, 50 });
            }
            {
                WHEN("the first child's flex-grow is set to 2");
                childTree1.setProperty("flex-grow", 2, nullptr);

                THEN("the first child's component covers the left 2 thirds of the parent's width");
                expect(item.getChild(0).getComponent().getBounds() == juce::Rectangle<int>{ 0, 0, 200, 50 });

                THEN("the second child's component covers the right third of the parent's width");
                expect(item.getChild(1).getComponent().getBounds() == juce::Rectangle<int>{ 200, 0, 100, 50 });
            }
        }
    }

    void testFlexItemSize()
    {
        beginTest("Test flex-item size");

        {
            GIVEN("a GUI item with a flex layout and a child with a specified size");
            juce::ValueTree tree{ "Component" };
            tree.setProperty("display", "flex", nullptr);
            jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

            juce::ValueTree childTree{ "Child" };
            childTree.setProperty("width", 89, nullptr);
            childTree.setProperty("height", 37, nullptr);
            auto child = std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(), childTree);
            item.addChild(std::move(child));

            {
                WHEN("the parent item's component is given a size of [200, 200]");
                item.getComponent().setSize(200, 200);

                THEN("the size of the child's component matches the specified size");
                expectEquals(item.getChild(0).getComponent().getWidth(), 89);
                expectEquals(item.getChild(0).getComponent().getHeight(), 37);
            }
        }
    }
};

//======================================================================================================================
static GuiItemTests guiItemTests;
