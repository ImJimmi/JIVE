#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridItem::GridItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , order{ tree, "order" }
    {
    }

    //==================================================================================================================
    GridItem::operator juce::GridItem()
    {
        juce::GridItem gridItem{ getComponent() };

        gridItem.order = order;

        return gridItem;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class GridItemUnitTest : public juce::UnitTest
{
public:
    GridItemUnitTest()
        : juce::UnitTest{ "jive::GridItem", "jive" }
    {
    }

    void runTest() final
    {
        testComponent();
        testOrder();
    }

private:
    std::unique_ptr<jive::GridItem> createGridItem(juce::ValueTree tree)
    {
        return std::make_unique<jive::GridItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                tree));
    }

    void testComponent()
    {
        beginTest("component");

        juce::ValueTree tree{ "Component" };
        auto item = createGridItem(tree);

        auto gridItem = static_cast<juce::GridItem>(*item);
        expect(gridItem.associatedComponent == &item->getComponent());
    }

    void testOrder()
    {
        beginTest("order");

        {
            juce::ValueTree tree{ "Component" };
            auto item = createGridItem(tree);

            auto gridItem = static_cast<juce::GridItem>(*item);
            expect(gridItem.order == 0);

            tree.setProperty("order", 123, nullptr);

            gridItem = static_cast<juce::GridItem>(*item);
            expect(gridItem.order == 123);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "order", 456 },
                },
            };
            auto item = createGridItem(tree);

            auto gridItem = static_cast<juce::GridItem>(*item);
            expect(gridItem.order == 456);
        }
    }
};

static GridItemUnitTest gridItemUnitTest;
#endif
