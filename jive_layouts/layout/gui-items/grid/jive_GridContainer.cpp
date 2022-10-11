#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , justifyItems{ state, "justify-items", juce::Grid::JustifyItems::stretch }
        , alignItems{ state, "align-items", juce::Grid::AlignItems::stretch }
        , justifyContent{ state, "justify-content", juce::Grid::JustifyContent::stretch }
        , alignContent{ state, "align-content", juce::Grid::AlignContent::stretch }
        , autoFlow{ state, "auto-flow", juce::Grid::AutoFlow::row }
        , templateColumns{ state, "template-columns" }
        , templateRows{ state, "template-rows" }
        , templateAreas{ state, "template-areas" }
        , autoRows{ state, "auto-rows", {} }
        , autoColumns{ state, "auto-columns", {} }
        , gap{ state, "gap" }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::grid));

        const auto setExplicitWidthAndHeight = [this]() {
            boxModel.setWidth(juce::jmax(boxModel.getWidth(), calculateMinimumContentWidth()));
            boxModel.setHeight(juce::jmax(boxModel.getHeight(), calculateMinimumContentHeight()));
        };

        justifyItems.onValueChange = setExplicitWidthAndHeight;
        alignItems.onValueChange = setExplicitWidthAndHeight;
        justifyContent.onValueChange = setExplicitWidthAndHeight;
        alignContent.onValueChange = setExplicitWidthAndHeight;
        autoFlow.onValueChange = setExplicitWidthAndHeight;
        templateColumns.onValueChange = setExplicitWidthAndHeight;
        templateRows.onValueChange = setExplicitWidthAndHeight;
        templateAreas.onValueChange = setExplicitWidthAndHeight;
        autoRows.onValueChange = setExplicitWidthAndHeight;
        autoColumns.onValueChange = setExplicitWidthAndHeight;
        gap.onValueChange = setExplicitWidthAndHeight;
    }

    //==================================================================================================================
    void GridContainer::updateLayout()
    {
        auto grid = getGrid();
        const auto bounds = boxModel.getContentBounds().toNearestInt();

        grid.performLayout(bounds);
    }

    //==================================================================================================================
    void GridContainer::addChild(std::unique_ptr<GuiItem> child)
    {
        GuiItemDecorator::addChild(std::move(child));

        boxModel.setWidth(juce::jmax(boxModel.getWidth(), calculateMinimumContentWidth()));
        boxModel.setHeight(juce::jmax(boxModel.getHeight(), calculateMinimumContentHeight()));
    }

    //==================================================================================================================
    GridContainer::operator juce::Grid()
    {
        return getGrid();
    }

    //==================================================================================================================
    void GridContainer::componentMovedOrResized(juce::Component& componentThatWasMovedOrResized,
                                                bool wasMoved,
                                                bool wasResized)
    {
        GuiItemDecorator::componentMovedOrResized(componentThatWasMovedOrResized, wasMoved, wasResized);

        if (!wasResized)
            return;

        updateLayout();
    }

    //==================================================================================================================
    void appendChildren(GuiItem& container, juce::Grid& grid)
    {
        for (auto& child : container)
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(&child))
            {
                if (auto* const gridItem = decoratedItem->toType<GridItem>())
                    grid.items.add(*gridItem);
            }
        }
    }

    juce::Grid GridContainer::getGrid()
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

        appendChildren(*this, grid);

        return grid;
    }

    juce::Grid GridContainer::getGridWithDummyItems() const
    {
        auto grid = const_cast<GridContainer*>(this)->getGrid();

        for (auto& gridItem : grid.items)
            gridItem.associatedComponent = nullptr;

        return grid;
    }

    void performDummyLayout(juce::Grid& grid)
    {
        grid.autoRows = juce::Grid::Px{ 1 };
        grid.autoColumns = juce::Grid::Px{ 1 };

        grid.performLayout(juce::Rectangle<int>{ 0, 0, 0, 0 });
    }

    float GridContainer::calculateMinimumContentWidth() const
    {
        auto grid = getGridWithDummyItems();
        performDummyLayout(grid);

        auto contentWidth = 0.f;

        for (const auto& gridItem : grid.items)
        {
            if (gridItem.currentBounds.getRight() > contentWidth)
                contentWidth = gridItem.currentBounds.getRight();
        }

        return contentWidth;
    }

    float GridContainer::calculateMinimumContentHeight() const
    {
        auto grid = getGridWithDummyItems();
        performDummyLayout(grid);

        auto contentHeight = 0.f;

        for (const auto& gridItem : grid.items)
        {
            if (gridItem.currentBounds.getBottom() > contentHeight)
                contentHeight = gridItem.currentBounds.getBottom();
        }

        return contentHeight;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
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
        testItems();
        testLayout();
        testAutoSize();
    }

private:
    std::unique_ptr<jive::GridContainer> createGridContainer(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        tree.setProperty("display", "grid", nullptr);

        return std::make_unique<jive::GridContainer>(interpreter.interpret(tree));
    }

    std::unique_ptr<jive::GridItem> createGridItem(jive::GuiItem* const parent)
    {
        return std::make_unique<jive::GridItem>(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                                juce::ValueTree{ "Component" },
                                                                                parent));
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

    void testItems()
    {
        beginTest("children");

        juce::ValueTree tree{ "Component" };
        auto item = createGridContainer(tree);

        auto grid = static_cast<juce::Grid>(*item);
        expect(grid.items.isEmpty());

        item->addChild(createGridItem(item.get()));
        grid = static_cast<juce::Grid>(*item);
        expect(grid.items.size() == 1);

        item->addChild(createGridItem(item.get()));
        item->addChild(createGridItem(item.get()));
        grid = static_cast<juce::Grid>(*item);
        expect(grid.items.size() == 3);
    }

    void testLayout()
    {
        beginTest("layout");

        juce::ValueTree tree{
            "Component",
            {
                { "template-columns", "1fr 1fr" },
                { "template-rows", "1fr" },
            },
        };
        auto item = createGridContainer(tree);
        item->addChild(createGridItem(item.get()));
        item->addChild(createGridItem(item.get()));

        item->getComponent().setSize(200, 200);
        expectGreaterThan(item->getChild(0).getComponent().getWidth(), 0);
        expectGreaterThan(item->getChild(1).getComponent().getWidth(), 0);
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        juce::ValueTree tree{
            "Component",
            {
                { "template-columns", "10px 10px" },
                { "template-rows", "10px 1fr" },
                { "gap", "5" },
            },
        };
        auto item = createGridContainer(tree);
        item->addChild(createGridItem(item.get()));
        item->addChild(createGridItem(item.get()));
        item->addChild(createGridItem(item.get()));
        item->addChild(createGridItem(item.get()));

        expectEquals(item->boxModel.getWidth(), 25.f);
        expectEquals(item->boxModel.getHeight(), 15.f);
    }
};

static GridContainerTest gridContainerTest;
#endif
