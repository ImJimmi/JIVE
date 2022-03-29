#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridItem::GridItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
    {
    }

    //==================================================================================================================
    GridItem::operator juce::GridItem()
    {
        juce::GridItem gridItem{ getComponent() };

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
};

static GridItemUnitTest gridItemUnitTest;
#endif
