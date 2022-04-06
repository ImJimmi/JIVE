#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , justifyItems{ tree, "justify-items", juce::Grid::JustifyItems::stretch }
        , alignItems{ tree, "align-items", juce::Grid::AlignItems::stretch }
    {
    }

    //==================================================================================================================
    GridContainer::operator juce::Grid()
    {
        juce::Grid grid;

        grid.justifyItems = justifyItems;
        grid.alignItems = alignItems;

        return grid;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class GridContainerTest : public juce::UnitTest
{
public:
    GridContainerTest()
        : juce::UnitTest{ "jive::GridContainer", "jive" }
    {
    }

    void runTest() override
    {
        testJustifyItems();
        testAlignItems();
    }

private:
    std::unique_ptr<jive::GridContainer> createGridContainer(juce::ValueTree tree)
    {
        return std::make_unique<jive::GridContainer>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                     tree));
    }

    void testJustifyItems()
    {
        beginTest("justify-items");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyItems == juce::Grid::JustifyItems::stretch);

        tree.setProperty("justify-items", "start", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyItems == juce::Grid::JustifyItems::start);

        tree.setProperty("justify-items", "end", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyItems == juce::Grid::JustifyItems::end);

        tree.setProperty("justify-items", "centre", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyItems == juce::Grid::JustifyItems::center);

        tree.setProperty("justify-items", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyItems == juce::Grid::JustifyItems::stretch);
    }

    void testAlignItems()
    {
        beginTest("align-items");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.alignItems == juce::Grid::AlignItems::stretch);

        tree.setProperty("align-items", "start", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignItems == juce::Grid::AlignItems::start);

        tree.setProperty("align-items", "end", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignItems == juce::Grid::AlignItems::end);

        tree.setProperty("align-items", "centre", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignItems == juce::Grid::AlignItems::center);

        tree.setProperty("align-items", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignItems == juce::Grid::AlignItems::stretch);
    }
};

static GridContainerTest gridContainerTest;
#endif
