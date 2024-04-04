#include "jive_GridContainer.h"

#include "jive_GridItem.h"

#include <jive_layouts/layout/gui-items/content/jive_Text.h>
#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

namespace jive
{
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem(std::move(itemToDecorate))
        , justifyItems{ state, "justify-items" }
        , alignItems{ state, "align-items" }
        , justifyContent{ state, "justify-content" }
        , alignContent{ state, "align-content" }
        , gridAutoFlow{ state, "grid-auto-flow" }
        , gridTemplateColumns{ state, "grid-template-columns" }
        , gridTemplateRows{ state, "grid-template-rows" }
        , gridTemplateAreas{ state, "grid-template-areas" }
        , gridAutoRows{ state, "grid-auto-rows" }
        , gridAutoColumns{ state, "grid-auto-columns" }
        , gap{ state, "gap" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::grid));

        static const juce::Grid defaultGrid;

        if (!justifyItems.exists())
            justifyItems = defaultGrid.justifyItems;
        if (!alignItems.exists())
            alignItems = defaultGrid.alignItems;
        if (!justifyContent.exists())
            justifyContent = defaultGrid.justifyContent;
        if (!alignContent.exists())
            alignContent = defaultGrid.alignContent;
        if (!gridAutoFlow.exists())
            gridAutoFlow = defaultGrid.autoFlow;
        if (!gridAutoRows.exists())
            gridAutoRows = defaultGrid.autoRows;
        if (!gridAutoColumns.exists())
            gridAutoColumns = defaultGrid.autoColumns;

        justifyItems.onValueChange = [this]() {
            layoutChanged();
        };
        alignItems.onValueChange = [this]() {
            layoutChanged();
        };
        justifyContent.onValueChange = [this]() {
            layoutChanged();
        };
        alignContent.onValueChange = [this]() {
            layoutChanged();
        };
        gridAutoFlow.onValueChange = [this]() {
            layoutChanged();
        };
        gridTemplateColumns.onValueChange = [this]() {
            layoutChanged();
        };
        gridTemplateRows.onValueChange = [this]() {
            layoutChanged();
        };
        gridTemplateAreas.onValueChange = [this]() {
            layoutChanged();
        };
        gridAutoRows.onValueChange = [this]() {
            layoutChanged();
        };
        gridAutoColumns.onValueChange = [this]() {
            layoutChanged();
        };
        gap.onValueChange = [this]() {
            layoutChanged();
        };
    }

    void GridContainer::layOutChildren()
    {
        if (layoutRecursionLock)
            return;

        const juce::ScopedValueSetter svs{ layoutRecursionLock, true };

        GuiItemDecorator::layOutChildren();

        const auto bounds = boxModel.getContentBounds().toNearestInt();

        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;
        do
        {
            changesDuringLayout = false;
            buildGrid(bounds, LayoutStrategy::real)
                .performLayout(bounds);
        }
        while (changesDuringLayout);
    }

    GridContainer::operator juce::Grid()
    {
        return buildGrid(boxModel.getContentBounds().toNearestInt(),
                         LayoutStrategy::real);
    }

    juce::Rectangle<float> GridContainer::calculateIdealSize(juce::Rectangle<float> constraints) const
    {
        auto integerConstraints = constraints.toNearestInt().withZeroOrigin();
        integerConstraints.setHeight(static_cast<int>(std::numeric_limits<juce::uint16>::max()));

        auto grid = const_cast<GridContainer&>(*this)
                        .buildGrid(integerConstraints, LayoutStrategy::dummy);
        grid.performLayout(integerConstraints);

        juce::Point extremities{ -1.0f, -1.0f };

        for (const auto& gridItem : grid.items)
        {
            const auto right = gridItem.currentBounds.getRight() + gridItem.margin.right;
            if (right > extremities.x)
                extremities.x = right;

            const auto bottom = gridItem.currentBounds.getBottom() + gridItem.margin.bottom;
            if (bottom > extremities.y)
                extremities.y = bottom;
        }

        return {
            extremities.x
                + boxModel
                      .getPadding()
                      .getLeftAndRight()
                + boxModel
                      .getBorder()
                      .getLeftAndRight(),
            extremities.y
                + boxModel
                      .getPadding()
                      .getTopAndBottom()
                + boxModel
                      .getBorder()
                      .getTopAndBottom(),
        };
    }

    static void appendChildren(GuiItem& container,
                               juce::Grid& grid,
                               juce::Rectangle<int> bounds,
                               LayoutStrategy strategy)
    {
        for (auto* child : container.getChildren())
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(child))
            {
                if (auto* const gridItem = decoratedItem->toType<GridItem>())
                    grid.items.add(gridItem->toJuceGridItem(bounds.toFloat(), strategy));
            }
        }
    }

    juce::Grid GridContainer::buildGrid(juce::Rectangle<int> bounds,
                                        LayoutStrategy strategy)
    {
        juce::Grid grid;

        grid.autoFlow = gridAutoFlow;
        grid.templateColumns = gridTemplateColumns;
        grid.templateRows = gridTemplateRows;
        grid.templateAreas = gridTemplateAreas;
        grid.autoRows = gridAutoRows;
        grid.autoColumns = gridAutoColumns;

        const auto gaps = gap.get();
        grid.rowGap = gaps.size() > 0 ? gaps.getUnchecked(0) : juce::Grid::Px{ 0 };
        grid.columnGap = gaps.size() > 1 ? gaps.getUnchecked(1) : grid.rowGap;

        appendChildren(*this, grid, bounds, strategy);

        switch (strategy)
        {
        case LayoutStrategy::real:
            grid.justifyItems = justifyItems;
            grid.alignItems = alignItems;
            grid.justifyContent = justifyContent;
            grid.alignContent = alignContent;
            break;
        case LayoutStrategy::dummy:
            grid.justifyItems = juce::Grid::JustifyItems::start;
            grid.alignItems = juce::Grid::AlignItems::start;
            grid.justifyContent = juce::Grid::JustifyContent::start;
            grid.alignContent = juce::Grid::AlignContent::start;

            for (auto& column : grid.templateColumns)
            {
                if (column.isFractional())
                    column = juce::Grid::TrackInfo{};
            }
            for (auto& row : grid.templateRows)
            {
                if (row.isFractional())
                    row = juce::Grid::TrackInfo{};
            }

            break;
        }

        return grid;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

static bool compare(const juce::Grid::TrackInfo& a, const juce::Grid::TrackInfo& b)
{
    return (a.isAuto() == b.isAuto())
        && (a.isFractional() == b.isFractional())
        && (a.isPixels() == b.isPixels())
        && (a.getStartLineName() == b.getStartLineName())
        && (a.getEndLineName() == b.getEndLineName())
        && (juce::approximatelyEqual(a.getSize(), b.getSize()));
}

static bool compare(const juce::Array<juce::Grid::TrackInfo>& a,
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
        testNestedText();
    }

private:
    void testJustifyItems()
    {
        beginTest("justify-items");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.justifyItems == juce::Grid::JustifyItems::stretch);

        state.setProperty("justify-items", "start", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyItems == juce::Grid::JustifyItems::start);

        state.setProperty("justify-items", "end", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyItems == juce::Grid::JustifyItems::end);

        state.setProperty("justify-items", "centre", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyItems == juce::Grid::JustifyItems::center);

        state.setProperty("justify-items", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyItems == juce::Grid::JustifyItems::stretch);
    }

    void testAlignItems()
    {
        beginTest("align-items");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.alignItems == juce::Grid::AlignItems::stretch);

        state.setProperty("align-items", "start", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignItems == juce::Grid::AlignItems::start);

        state.setProperty("align-items", "end", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignItems == juce::Grid::AlignItems::end);

        state.setProperty("align-items", "centre", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignItems == juce::Grid::AlignItems::center);

        state.setProperty("align-items", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignItems == juce::Grid::AlignItems::stretch);
    }

    void testJustifyContent()
    {
        beginTest("justify-content");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::stretch);

        state.setProperty("justify-content", "start", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::start);

        state.setProperty("justify-content", "end", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::end);

        state.setProperty("justify-content", "centre", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::center);

        state.setProperty("justify-content", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::stretch);

        state.setProperty("justify-content", "space-around", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceAround);

        state.setProperty("justify-content", "space-between", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceBetween);

        state.setProperty("justify-content", "space-evenly", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.justifyContent == juce::Grid::JustifyContent::spaceEvenly);
    }

    void testAlignContent()
    {
        beginTest("align-content");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::stretch);

        state.setProperty("align-content", "start", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::start);

        state.setProperty("align-content", "end", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::end);

        state.setProperty("align-content", "centre", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::center);

        state.setProperty("align-content", "stretch", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::stretch);

        state.setProperty("align-content", "space-around", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::spaceAround);

        state.setProperty("align-content", "space-between", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::spaceBetween);

        state.setProperty("align-content", "space-evenly", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.alignContent == juce::Grid::AlignContent::spaceEvenly);
    }

    void testAutoFlow()
    {
        beginTest("grid-auto-flow");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::row);

        state.setProperty("grid-auto-flow", "column", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::column);

        state.setProperty("grid-auto-flow", "row", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::row);

        state.setProperty("grid-auto-flow", "row dense", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::rowDense);

        state.setProperty("grid-auto-flow", "column dense", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::columnDense);
    }

    void testTemplateColumns()
    {
        beginTest("grid-template-columns");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Grid{}.templateColumns));

        state.setProperty("grid-template-columns", "1fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 1 } }));

        state.setProperty("grid-template-columns", "4px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 4 } }));

        state.setProperty("grid-template-columns", "24", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 24 } }));

        state.setProperty("grid-template-columns", "auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        state.setProperty("grid-template-columns", "10 30 40", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 10 },
                           juce::Grid::Px{ 30 },
                           juce::Grid::Px{ 40 },
                       }));

        state.setProperty("grid-template-columns", "auto 2fr 99px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::TrackInfo{},
                           juce::Grid::Fr{ 2 },
                           juce::Grid::Px{ 99 },
                       }));
    }

    void testTemplateRows()
    {
        beginTest("grid-template-rows");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Grid{}.templateRows));

        state.setProperty("grid-template-rows", "5fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 5 } }));

        state.setProperty("grid-template-rows", "8px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 8 } }));

        state.setProperty("grid-template-rows", "17", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 17 } }));

        state.setProperty("grid-template-rows", "auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        state.setProperty("grid-template-rows", "1 313 67", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 1 },
                           juce::Grid::Px{ 313 },
                           juce::Grid::Px{ 67 },
                       }));

        state.setProperty("grid-template-rows", "78px 3fr auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 78 },
                           juce::Grid::Fr{ 3 },
                           juce::Grid::TrackInfo{},
                       }));
    }

    void testTemplateAreas()
    {
        beginTest("grid-template-areas");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(grid.templateAreas == juce::Grid{}.templateAreas);

        state.setProperty("grid-template-areas", "a b c-d e.f", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.templateAreas == juce::StringArray{ "a", "b", "c-d", "e.f" });
    }

    void testAutoRows()
    {
        beginTest("grid-auto-rows");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(compare(grid.autoRows, juce::Grid::TrackInfo{}));

        state.setProperty("grid-auto-rows", "100px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoRows, juce::Grid::Px{ 100 }));

        state.setProperty("grid-auto-rows", "13fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoRows, juce::Grid::Fr{ 13 }));
    }

    void testAutoColumns()
    {
        beginTest("grid-auto-columns");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expect(compare(grid.autoColumns, juce::Grid::TrackInfo{}));

        state.setProperty("grid-auto-columns", "1.23px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoColumns, juce::Grid::Px{ 1.23L }));

        state.setProperty("grid-auto-columns", "15fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoColumns, juce::Grid::Fr{ 15 }));
    }

    void testGap()
    {
        beginTest("gap");

        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                 .toType<jive::GridContainer>());
        expectEquals(grid.columnGap.pixels, 0.0L);
        expectEquals(grid.rowGap.pixels, 0.0L);

        state.setProperty("gap", "10", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expectEquals(grid.columnGap.pixels, 10.0L);
        expectEquals(grid.rowGap.pixels, 10.0L);

        state.setProperty("gap", "5 33", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expectEquals(grid.columnGap.pixels, 33.0L);
        expectEquals(grid.rowGap.pixels, 5.0L);
    }

    void testItems()
    {
        beginTest("children");

        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            const auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                           .toType<jive::GridContainer>());
            expect(grid.items.isEmpty());
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            const auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                           .toType<jive::GridContainer>());
            expectEquals(grid.items.size(), 1);
        }
        {
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{ "Component" },
                    juce::ValueTree{ "Component" },
                    juce::ValueTree{ "Component" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            const auto grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                                           .toType<jive::GridContainer>());
            expect(grid.items.size() == 3);
        }
    }

    void testLayout()
    {
        beginTest("layout");

        juce::ValueTree state{
            "Component",
            {
                { "display", "grid" },
                { "width", 200 },
                { "height", 200 },
                { "grid-template-columns", "1fr 1fr" },
                { "grid-template-rows", "1fr" },
            },
            {
                juce::ValueTree{ "Component" },
                juce::ValueTree{ "Component" },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        expectGreaterThan(item->getChildren()[0]->getComponent()->getWidth(), 0);
        expectGreaterThan(item->getChildren()[1]->getComponent()->getWidth(), 0);
    }

    void testAutoSize()
    {
        beginTest("auto-size");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "align-items", "flex-start" },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "display", "grid" },
                        { "grid-template-columns", "10px 10px" },
                        { "grid-template-rows", "10px 1fr" },
                        { "gap", "5" },
                    },
                    {
                        juce::ValueTree{ "Component" },
                        juce::ValueTree{ "Component" },
                        juce::ValueTree{ "Component" },
                        juce::ValueTree{ "Component" },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        const auto& boxModel = jive::boxModel(item);
        expectEquals(boxModel.getWidth(), 25.f);
        expectEquals(boxModel.getHeight(), 15.f);
    }

    void testNestedText()
    {
        beginTest("nested text");

        juce::ValueTree state{
            "Window",
            {
                { "width", 200 },
                { "height", 500 },
                { "align-items", "flex-start" },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "display", "grid" },
                        { "grid-template-columns", "1fr 1fr" },
                        { "grid-template-rows", "1fr 1fr" },
                    },
                    {
                        juce::ValueTree{ "Text", { { "text", "hello" } } },
                        juce::ValueTree{ "Text", { { "text", "world" } } },
                        juce::ValueTree{ "Text", { { "text", "foo" } } },
                        juce::ValueTree{ "Text", { { "text", "bar" } } },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        const auto window = interpreter.interpret(state);
        auto& container = *window->getChildren()[0];
        const auto& boxModel = jive::boxModel(container);
        const auto font = dynamic_cast<jive::GuiItemDecorator*>(container.getChildren()[0])
                              ->toType<jive::Text>()
                              ->getTextComponent()
                              .getFont();

        const auto expectedWidth = std::ceil(font.getStringWidthFloat("hello")) + std::ceil(font.getStringWidthFloat("world"));
        expectEquals(boxModel.getContentBounds().getWidth(), expectedWidth);

        const auto expectedHeight = std::ceil(font.getHeight()) * 2.0f;
        expectEquals(boxModel.getContentBounds().getHeight(), expectedHeight);

        container.state.getChild(0).setProperty("text", "hello world lorum ipsum dolor etc...", nullptr);
        expectEquals(boxModel.getContentBounds().getWidth(), static_cast<float>(state["width"]));
        expectEquals(jive::boxModel(*container.getChildren()[0]).getHeight(), font.getHeight() * 2.0f);
    }
};

static GridContainerTest gridContainerTest;
#endif
