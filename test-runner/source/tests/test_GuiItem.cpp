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
            expectEquals(item.getID(), juce::String{});
        }

        {
            WHEN("the value-tree being used by the item has its 'id' property set to '123'");
            tree.setProperty("id", "123", nullptr);

            THEN("the item's ID is '123'.");
            expectEquals(item.getID(), juce::String{ "123" });
        }
        {
            WHEN("the value-tree begin used by the item has its 'id' property set to '456'");
            tree.setProperty("id", "456", nullptr);

            THEN("the item's ID is '456'.");
            expectEquals(item.getID(), juce::String{ "456" });
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
            tree.setProperty("display",
                             juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex),
                             nullptr);

            THEN("the item's display type is 'flex'");
            expect(item.getDisplay() == jive::GuiItem::Display::flex);
        }
        {
            WHEN("the value-tree being used by the item has its 'display' property set to 'grid'");
            tree.setProperty("display",
                             juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::grid),
                             nullptr);

            THEN("the item's display type is 'grid'");
            expect(item.getDisplay() == jive::GuiItem::Display::grid);
        }
    }
};

//======================================================================================================================
static GuiItemTests guiItemTests;
