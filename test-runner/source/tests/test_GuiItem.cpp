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
