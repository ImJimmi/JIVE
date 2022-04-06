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
        , templateColumns{ tree, "template-columns" }
        , templateRows{ tree, "template-rows" }
        , templateAreas{ tree, "template-areas" }
        , autoRows{ tree, "auto-rows" }
        , autoColumns{ tree, "auto-columns" }
        , gap{ tree, "gap" }
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

        grid.templateColumns = templateColumns;
        grid.templateRows = templateRows;
        grid.templateAreas = templateAreas;
        grid.autoRows = autoRows;
        grid.autoColumns = autoColumns;

        const auto gaps = gap.get();
        grid.rowGap = gaps.size() > 0 ? gaps.getUnchecked(0) : juce::Grid::Px{ 0 };
        grid.columnGap = gaps.size() > 1 ? gaps.getUnchecked(1) : grid.rowGap;

        return grid;
    }
} // namespace jive

//======================================================================================================================
bool compare(const juce::Grid::TrackInfo& a, const juce::Grid::TrackInfo& b)
{
    return (a.isAuto() == b.isAuto())
        && (a.isFractional() == b.isFractional())
        && (a.isPixels() == b.isPixels())
        && (a.getStartLineName() == b.getStartLineName())
        && (a.getEndLineName() == b.getEndLineName())
        && (a.getSize() == b.getSize());
}

bool compare(const juce::Array<juce::Grid::TrackInfo>& a,
             const juce::Array<juce::Grid::TrackInfo>& b)
{
    if (a.size() != b.size())
        return false;

    for (auto i = 0; i < a.size(); i++)
    {
        if (!compare(a[i], b[i]))
            return false;
    }

    return true;
}

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
        testTemplateColumns();
        testTemplateRows();
        testTemplateAreas();
        testAutoRows();
        testAutoColumns();
        testGap();
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

    void testTemplateColumns()
    {
        beginTest("template-columns");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.templateColumns.isEmpty());

        tree.setProperty("template-columns", "1fr", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 1 } }));

        tree.setProperty("template-columns", "4px", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 4 } }));

        tree.setProperty("template-columns", "24", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 24 } }));

        tree.setProperty("template-columns", "auto", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        tree.setProperty("template-columns", "10 30 40", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 10 },
                           juce::Grid::Px{ 30 },
                           juce::Grid::Px{ 40 },
                       }));

        tree.setProperty("template-columns", "auto 2fr 99px", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateColumns,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::TrackInfo{},
                           juce::Grid::Fr{ 2 },
                           juce::Grid::Px{ 99 },
                       }));
    }

    void testTemplateRows()
    {
        beginTest("template-rows");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.templateRows.isEmpty());

        tree.setProperty("template-rows", "5fr", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 5 } }));

        tree.setProperty("template-rows", "8px", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 8 } }));

        tree.setProperty("template-rows", "17", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 17 } }));

        tree.setProperty("template-rows", "auto", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        tree.setProperty("template-rows", "1 313 67", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 1 },
                           juce::Grid::Px{ 313 },
                           juce::Grid::Px{ 67 },
                       }));

        tree.setProperty("template-rows", "78px 3fr auto", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 78 },
                           juce::Grid::Fr{ 3 },
                           juce::Grid::TrackInfo{},
                       }));
    }

    void testTemplateAreas()
    {
        beginTest("template-areas");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.templateAreas.isEmpty());

        tree.setProperty("template-areas", "a b c", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.templateAreas == juce::StringArray{ "a", "b", "c" });
    }

    void testAutoRows()
    {
        beginTest("auto-rows");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoRows, juce::Grid::TrackInfo{}));

        tree.setProperty("auto-rows", "100px", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoRows, juce::Grid::Px{ 100 }));

        tree.setProperty("auto-rows", "13fr", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoRows, juce::Grid::Fr{ 13 }));
    }

    void testAutoColumns()
    {
        beginTest("auto-columns");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoColumns, juce::Grid::TrackInfo{}));

        tree.setProperty("auto-columns", "1.23px", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoColumns, juce::Grid::Px{ 1.23L }));

        tree.setProperty("auto-columns", "15fr", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(compare(grid.autoColumns, juce::Grid::Fr{ 15 }));
    }

    void testGap()
    {
        beginTest("gap");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.columnGap.pixels == 0);
        expect(grid.rowGap.pixels == 0);

        tree.setProperty("gap", "10", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.columnGap.pixels == 10);
        expect(grid.rowGap.pixels == 10);

        tree.setProperty("gap", "5 33", nullptr);
        grid = static_cast<juce::Grid>(*item);
        expect(grid.columnGap.pixels == 33);
        expect(grid.rowGap.pixels == 5);
    }
};

static GridContainerTest gridContainerTest;
#endif
