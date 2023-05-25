#include <jive_layouts/jive_layouts.h>

namespace jive
{
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem(std::move(itemToDecorate))
        , justifyItems{ state, "justify-items", juce::Grid{}.justifyItems }
        , alignItems{ state, "align-items", juce::Grid{}.alignItems }
        , justifyContent{ state, "justify-content", juce::Grid{}.justifyContent }
        , alignContent{ state, "align-content", juce::Grid{}.alignContent }
        , autoFlow{ state, "auto-flow", juce::Grid{}.autoFlow }
        , templateColumns{ state, "template-columns", juce::Grid{}.templateColumns }
        , templateRows{ state, "template-rows", juce::Grid{}.templateRows }
        , templateAreas{ state, "template-areas", juce::Grid{}.templateAreas }
        , autoRows{ state, "auto-rows", juce::Grid{}.autoRows }
        , autoColumns{ state, "auto-columns", juce::Grid{}.autoColumns }
        , gap{ state, "gap" }
        , boxModel{ toType<CommonGuiItem>()->boxModel }
    {
        jassert(state.hasProperty("display"));
        jassert(state["display"] == juce::VariantConverter<Display>::toVar(Display::grid));

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
        autoFlow.onValueChange = [this]() {
            layoutChanged();
        };
        templateColumns.onValueChange = [this]() {
            layoutChanged();
        };
        templateRows.onValueChange = [this]() {
            layoutChanged();
        };
        templateAreas.onValueChange = [this]() {
            layoutChanged();
        };
        autoRows.onValueChange = [this]() {
            layoutChanged();
        };
        autoColumns.onValueChange = [this]() {
            layoutChanged();
        };
        gap.onValueChange = [this]() {
            layoutChanged();
        };
    }

    void GridContainer::layOutChildren()
    {
        const auto bounds = boxModel.getContentBounds().toNearestInt();

        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
            return;

        buildGrid().performLayout(bounds);
    }

    GridContainer::operator juce::Grid()
    {
        return buildGrid();
    }

    juce::Rectangle<float> GridContainer::calculateIdealSize(juce::Rectangle<float>) const
    {
        const auto grid = buildGridWithDummyItems();
        juce::Point<float> extremities{ -1.0f, -1.0f };

        for (const auto& gridItem : grid.items)
        {
            const auto right = gridItem.currentBounds.getRight() + gridItem.margin.right;
            const auto bottom = gridItem.currentBounds.getBottom() + gridItem.margin.bottom;

            if (right > extremities.x)
                extremities.x = right;
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

    void appendChildren(GuiItem& container, juce::Grid& grid)
    {
        for (auto* child : container.getChildren())
        {
            if (auto* const decoratedItem = dynamic_cast<GuiItemDecorator*>(child))
            {
                if (auto* const gridItem = decoratedItem->toType<GridItem>())
                    grid.items.add(*gridItem);
            }
        }
    }

    juce::Grid GridContainer::buildGrid()
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

    juce::Grid GridContainer::buildGridWithDummyItems() const
    {
        auto grid = const_cast<GridContainer*>(this)->buildGrid();

        grid.autoRows = juce::Grid::Px{ 1 };
        grid.autoColumns = juce::Grid::Px{ 1 };

        for (auto& gridItem : grid.items)
            gridItem.associatedComponent = nullptr;

        grid.performLayout(juce::Rectangle{ 1, 1 });

        return grid;
    }
} // namespace jive

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
        beginTest("auto-flow");

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

        state.setProperty("auto-flow", "column", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::column);

        state.setProperty("auto-flow", "row", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::row);

        state.setProperty("auto-flow", "row dense", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::rowDense);

        state.setProperty("auto-flow", "column dense", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.autoFlow == juce::Grid::AutoFlow::columnDense);
    }

    void testTemplateColumns()
    {
        beginTest("template-columns");

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
        expect(grid.templateColumns.isEmpty());

        state.setProperty("template-columns", "1fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 1 } }));

        state.setProperty("template-columns", "4px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 4 } }));

        state.setProperty("template-columns", "24", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 24 } }));

        state.setProperty("template-columns", "auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        state.setProperty("template-columns", "10 30 40", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateColumns,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 10 },
                           juce::Grid::Px{ 30 },
                           juce::Grid::Px{ 40 },
                       }));

        state.setProperty("template-columns", "auto 2fr 99px", nullptr);
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
        beginTest("template-rows");

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
        expect(grid.templateRows.isEmpty());

        state.setProperty("template-rows", "5fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Fr{ 5 } }));

        state.setProperty("template-rows", "8px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 8 } }));

        state.setProperty("template-rows", "17", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::Px{ 17 } }));

        state.setProperty("template-rows", "auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows, juce::Array<juce::Grid::TrackInfo>{ juce::Grid::TrackInfo{} }));

        state.setProperty("template-rows", "1 313 67", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 1 },
                           juce::Grid::Px{ 313 },
                           juce::Grid::Px{ 67 },
                       }));

        state.setProperty("template-rows", "78px 3fr auto", nullptr);
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
        beginTest("template-areas");

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
        expect(grid.templateAreas.isEmpty());

        state.setProperty("template-areas", "a b c-d e.f", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.templateAreas == juce::StringArray{ "a", "b", "c-d", "e.f" });
    }

    void testAutoRows()
    {
        beginTest("auto-rows");

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

        state.setProperty("auto-rows", "100px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoRows, juce::Grid::Px{ 100 }));

        state.setProperty("auto-rows", "13fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoRows, juce::Grid::Fr{ 13 }));
    }

    void testAutoColumns()
    {
        beginTest("auto-columns");

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

        state.setProperty("auto-columns", "1.23px", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.autoColumns, juce::Grid::Px{ 1.23L }));

        state.setProperty("auto-columns", "15fr", nullptr);
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
        expect(grid.columnGap.pixels == 0);
        expect(grid.rowGap.pixels == 0);

        state.setProperty("gap", "10", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.columnGap.pixels == 10);
        expect(grid.rowGap.pixels == 10);

        state.setProperty("gap", "5 33", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(grid.columnGap.pixels == 33);
        expect(grid.rowGap.pixels == 5);
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
                { "template-columns", "1fr 1fr" },
                { "template-rows", "1fr" },
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
                        { "template-columns", "10px 10px" },
                        { "template-rows", "10px 1fr" },
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
        expectEquals(jive::BoxModel{ item.state }.getWidth(), 25.f);
        expectEquals(jive::BoxModel{ item.state }.getHeight(), 15.f);
    }
};

static GridContainerTest gridContainerTest;
#endif
