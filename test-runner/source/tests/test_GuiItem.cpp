#include <jive/jive.h>

//======================================================================================================================
class GuiItemTests : public juce::UnitTest
{
public:
    //==================================================================================================================
    GuiItemTests()
        : juce::UnitTest{ "jive::GuiItem" }
    {
    }

    //==================================================================================================================
    void runTest() final
    {
        testComponentIDs();
        testDisplayProperty();
    }

private:
    //==================================================================================================================
    void testComponentIDs()
    {
        beginTest("Test Component IDs");

        logMessage("    Given a GUI item,");
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        {
            logMessage("      when the value-tree being used by the item has its 'id' property set to '123',");
            tree.setProperty("id", "123", nullptr);

            logMessage("        then the component's ID is '123'.");
            expectEquals(item.getComponent().getComponentID(), juce::String{ "123" });
        }
        {
            logMessage("      And when the value-tree begin used by the item has its 'id' property set to '456',");
            tree.setProperty("id", "456", nullptr);

            logMessage("        then the component's ID is '456'.");
            expectEquals(item.getComponent().getComponentID(), juce::String{ "456" });
        }
    }

    void testDisplayProperty()
    {
        beginTest("Test display types");

        logMessage("    Given a GUI item,");
        juce::ValueTree tree{ "Component" };
        jive::GuiItem item{ std::make_unique<juce::Component>(), tree };

        {
            logMessage("      when the value-tree being used by the item has its 'display' property set to 'flex',");
            tree.setProperty("display",
                             juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex),
                             nullptr);

            logMessage("        then the item's display type is 'flex'.");
            expect(item.getDisplay() == jive::GuiItem::Display::flex);
        }
        {
            logMessage("      when the value-tree being used by the item has its 'display' property set to 'grid',");
            tree.setProperty("display",
                             juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::grid),
                             nullptr);

            logMessage("        then the item's display type is 'grid'.");
            expect(item.getDisplay() == jive::GuiItem::Display::grid);
        }
    }
};

//======================================================================================================================
static GuiItemTests guiItemTests;
