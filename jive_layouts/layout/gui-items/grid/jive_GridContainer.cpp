#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , justifyItems{ tree, "justify-items", juce::Grid::JustifyItems::stretch }
        , alignItems{ tree, "align-items", juce::Grid::AlignItems::stretch }
        , justifyContent{ tree, "justify-content", juce::Grid::JustifyContent::stretch }
        , alignContent{ tree, "align-content", juce::Grid::AlignContent::stretch }
        , autoFlow{ tree, "auto-flow", juce::Grid::AutoFlow::row }
    {
    }

    //==================================================================================================================
    GridContainer::operator juce::Grid()
    {
        juce::Grid grid;

        grid.justifyItems = justifyItems;
        grid.alignItems = alignItems;
        grid.justifyContent = justifyContent;
        grid.alignContent = alignContent;
        grid.autoFlow = autoFlow;

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
        testJustifyContent();
        testAlignContent();
        testAutoFlow();
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

    void testJustifyContent()
    {
        beginTest("justify-content");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::stretch);

        tree.setProperty("justify-content", "start", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::start);

        tree.setProperty("justify-content", "end", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::end);

        tree.setProperty("justify-content", "centre", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::center);

        tree.setProperty("justify-content", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::stretch);

        tree.setProperty("justify-content", "space-around", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceAround);

        tree.setProperty("justify-content", "space-between", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceBetween);

        tree.setProperty("justify-content", "space-evenly", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceEvenly);
    }

    void testAlignContent()
    {
        beginTest("align-content");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::stretch);

        tree.setProperty("align-content", "start", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::start);

        tree.setProperty("align-content", "end", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::end);

        tree.setProperty("align-content", "centre", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::center);

        tree.setProperty("align-content", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::stretch);

        tree.setProperty("align-content", "space-around", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::spaceAround);

        tree.setProperty("align-content", "space-between", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::spaceBetween);

        tree.setProperty("align-content", "space-evenly", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.alignContent == juce::Grid::AlignContent::spaceEvenly);
    }

    void testAutoFlow()
    {
        beginTest("auto-flow");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.autoFlow == juce::Grid::AutoFlow::row);

        tree.setProperty("auto-flow", "column", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.autoFlow == juce::Grid::AutoFlow::column);

        tree.setProperty("auto-flow", "row", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.autoFlow == juce::Grid::AutoFlow::row);

        tree.setProperty("auto-flow", "row dense", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.autoFlow == juce::Grid::AutoFlow::rowDense);

        tree.setProperty("auto-flow", "column dense", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.autoFlow == juce::Grid::AutoFlow::columnDense);
    }
};

static GridContainerTest gridContainerTest;
#endif
