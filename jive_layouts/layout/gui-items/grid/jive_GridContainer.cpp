#include "jive_GridContainer.h"

#include "jive_GridItem.h"

#include <jive_layouts/layout/gui-items/content/jive_Text.h>
#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

namespace jive
{
    GridContainer::GridContainer(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem(std::move(itemToDecorate))
        , justifyItems{ state, ids::justifyItems() }
        , alignItems{ state, ids::alignItems() }
        , justifyContent{ state, ids::justifyContent() }
        , alignContent{ state, ids::alignContent() }
        , gridAutoFlow{ state, ids::gridAutoFlow() }
        , gridTemplateColumns{ state, ids::gridTemplateColumns() }
        , gridTemplateRows{ state, ids::gridTemplateRows() }
        , gridTemplateAreas{ state, ids::gridTemplateAreas() }
        , gridAutoRows{ state, ids::gridAutoRows() }
        , gridAutoColumns{ state, ids::gridAutoColumns() }
        , gap{ state, ids::gap() }
    {
        jassert(state.hasProperty(ids::display()));
        jassert(state[ids::display()] == juce::VariantConverter<Display>::toVar(Display::grid));

        justifyItems.onValueChange = [this] {
            callLayoutChildrenWithRecursionLock();
        };
        alignItems.onValueChange = [this] {
            callLayoutChildrenWithRecursionLock();
        };
        justifyContent.onValueChange = [this] {
            callLayoutChildrenWithRecursionLock();
        };
        alignContent.onValueChange = [this] {
            callLayoutChildrenWithRecursionLock();
        };
        gridAutoFlow.onValueChange = [this] {
            updateIdealSize();
        };
        gridTemplateColumns.onValueChange = [this] {
            updateIdealSize();
        };
        gridTemplateColumns.onTransitionProgressed = [this] {
            updateIdealSize();
        };
        gridTemplateRows.onValueChange = [this] {
            updateIdealSize();
        };
        gridTemplateRows.onTransitionProgressed = [this] {
            updateIdealSize();
        };
        gridTemplateAreas.onValueChange = [this] {
            updateIdealSize();
        };
        gridAutoRows.onValueChange = [this] {
            updateIdealSize();
        };
        gridAutoColumns.onValueChange = [this] {
            updateIdealSize();
        };
        gap.onValueChange = [this] {
            updateIdealSize();
        };
        gap.onTransitionProgressed = [this] {
            updateIdealSize();
        };
    }

    GridContainer::~GridContainer() = default;

    static juce::Array<juce::GridItem> layOutWithIdealSizes(juce::Grid grid,
                                                            juce::Rectangle<int> bounds,
                                                            juce::GridItem::JustifySelf justifySelf,
                                                            juce::GridItem::AlignSelf alignSelf)
    {
        for (auto& item : grid.items)
        {
            item.associatedComponent = nullptr;
            item.justifySelf = justifySelf;
            item.alignSelf = alignSelf;

            if (item.width < 0.0f && item.minWidth > 0.0f)
                item.width = item.minWidth;
            if (item.height < 0.0f && item.minHeight > 0.0f)
                item.height = item.minHeight;
        }

        grid.performLayout(bounds);
        return grid.items;
    }

    // juce::Grid sizes auto tracks from each item's width and height, ignoring
    // the minimums that hold their ideal sizes, so the areas are measured with
    // ideal sizes made explicit: once aligned to the start of each area, and
    // once to the end.
    static juce::Array<juce::Rectangle<float>> calculateItemAreas(const juce::Grid& grid,
                                                                  juce::Rectangle<int> bounds)
    {
        const auto startAligned = layOutWithIdealSizes(grid,
                                                       bounds,
                                                       juce::GridItem::JustifySelf::start,
                                                       juce::GridItem::AlignSelf::start);
        const auto endAligned = layOutWithIdealSizes(grid,
                                                     bounds,
                                                     juce::GridItem::JustifySelf::end,
                                                     juce::GridItem::AlignSelf::end);

        juce::Array<juce::Rectangle<float>> areas;

        for (auto i = 0; i < grid.items.size(); i++)
        {
            const auto& margin = grid.items.getReference(i).margin;
            const auto& start = startAligned.getReference(i).currentBounds;
            const auto& end = endAligned.getReference(i).currentBounds;

            areas.add(juce::Rectangle<float>::leftTopRightBottom(std::round(start.getX() - margin.left),
                                                                 std::round(start.getY() - margin.top),
                                                                 std::round(end.getRight() + margin.right),
                                                                 std::round(end.getBottom() + margin.bottom)));
        }

        return areas;
    }

    template <typename Alignment>
    [[nodiscard]] static float constrainSize(float available, float size, float minSize, float maxSize, Alignment alignment)
    {
        const auto autoSize = alignment == Alignment::stretch ? available : minSize;
        const auto preferred = size < 0.0f ? autoSize : size;
        return juce::jmax(minSize, maxSize < 0.0f ? preferred : juce::jmin(maxSize, preferred));
    }

    template <typename Alignment>
    [[nodiscard]] static float alignPosition(float availableStart, float availableSize, float size, Alignment alignment)
    {
        switch (alignment)
        {
        case Alignment::end:
            return availableStart + availableSize - size;
        case Alignment::center:
            return availableStart + (availableSize - size) / 2.0f;
        case Alignment::start:
        case Alignment::stretch:
            break;
        }

        return availableStart;
    }

    [[nodiscard]] static juce::Rectangle<float> alignWithinArea(const juce::GridItem& item,
                                                                const juce::Grid& grid,
                                                                juce::Rectangle<float> area)
    {
        const auto justify = item.justifySelf == juce::GridItem::JustifySelf::autoValue
                               ? grid.justifyItems
                               : static_cast<juce::Grid::JustifyItems>(item.justifySelf);
        const auto align = item.alignSelf == juce::GridItem::AlignSelf::autoValue
                             ? grid.alignItems
                             : static_cast<juce::Grid::AlignItems>(item.alignSelf);

        const auto available = juce::BorderSize<float>{ item.margin.top, item.margin.left, item.margin.bottom, item.margin.right }
                                   .subtractedFrom(area);
        const auto width = constrainSize(available.getWidth(), item.width, item.minWidth, item.maxWidth, justify);
        const auto height = constrainSize(available.getHeight(), item.height, item.minHeight, item.maxHeight, align);

        return {
            std::round(alignPosition(available.getX(), available.getWidth(), width, justify)),
            std::round(alignPosition(available.getY(), available.getHeight(), height, align)),
            std::round(width),
            std::round(height),
        };
    }

    static void layOutItems(const juce::Grid& grid, juce::Rectangle<int> bounds)
    {
        const auto areas = calculateItemAreas(grid, bounds);

        for (auto i = 0; i < grid.items.size(); i++)
        {
            const auto& item = grid.items.getReference(i);

            if (item.associatedComponent != nullptr)
                item.associatedComponent->setBounds(alignWithinArea(item, grid, areas[i]).toNearestIntEdges());
        }
    }

    [[nodiscard]] static juce::Grid::TrackInfo withSize(const juce::Grid::TrackInfo& track, float size)
    {
        return juce::Grid::TrackInfo{
            track.getStartLineName(),
            juce::Grid::Px{ size },
            track.getEndLineName(),
        };
    }

    // Lays the items out in tracks that are all the same known size, so the
    // tracks each item is in can be told from where it ends up.
    [[nodiscard]] static juce::Array<juce::Rectangle<int>> findTracksOfItems(juce::Grid grid)
    {
        for (auto* tracks : { &grid.templateColumns, &grid.templateRows })
        {
            for (auto& track : *tracks)
                track = withSize(track, 1.0f);
        }

        grid.autoColumns = juce::Grid::TrackInfo{ juce::Grid::Px{ 1 } };
        grid.autoRows = juce::Grid::TrackInfo{ juce::Grid::Px{ 1 } };

        for (auto& item : grid.items)
        {
            item.width = juce::GridItem::notAssigned;
            item.height = juce::GridItem::notAssigned;
            item.minWidth = 0.0f;
            item.minHeight = 0.0f;
            item.maxWidth = juce::GridItem::notAssigned;
            item.maxHeight = juce::GridItem::notAssigned;
            item.margin = juce::GridItem::Margin{};
            item.justifySelf = juce::GridItem::JustifySelf::stretch;
            item.alignSelf = juce::GridItem::AlignSelf::stretch;
        }

        grid.performLayout({ std::numeric_limits<juce::uint16>::max(), std::numeric_limits<juce::uint16>::max() });

        const auto columnStride = 1.0f + static_cast<float>(grid.columnGap.pixels);
        const auto rowStride = 1.0f + static_cast<float>(grid.rowGap.pixels);
        juce::Array<juce::Rectangle<int>> tracks;

        for (const auto& item : grid.items)
        {
            const auto& bounds = item.currentBounds;
            tracks.add(juce::Rectangle<int>::leftTopRightBottom(juce::roundToInt(bounds.getX() / columnStride),
                                                                juce::roundToInt(bounds.getY() / rowStride),
                                                                juce::roundToInt((bounds.getRight() - 1.0f) / columnStride) + 1,
                                                                juce::roundToInt((bounds.getBottom() - 1.0f) / rowStride) + 1));
        }

        return tracks;
    }

    // juce::Grid stretches fractional tracks to fill whatever space they're
    // given, so to be measured they're made a fixed size instead. Like in CSS,
    // the space they'd share is what it takes for the biggest content in any
    // of them to fit, relative to their fractions.
    static void sizeFractionalTracksToContent(juce::Grid& grid)
    {
        const auto tracksOfItems = findTracksOfItems(grid);
        auto columnFraction = 0.0f;
        auto rowFraction = 0.0f;

        for (auto i = 0; i < grid.items.size(); i++)
        {
            const auto& item = grid.items.getReference(i);
            const auto& tracks = tracksOfItems.getReference(i);

            if (tracks.getWidth() == 1
                && juce::isPositiveAndBelow(tracks.getX(), grid.templateColumns.size())
                && grid.templateColumns.getReference(tracks.getX()).isFractional())
            {
                const auto width = juce::jmax(item.width, item.minWidth) + item.margin.left + item.margin.right;
                columnFraction = juce::jmax(columnFraction, width / grid.templateColumns.getReference(tracks.getX()).getSize());
            }

            if (tracks.getHeight() == 1
                && juce::isPositiveAndBelow(tracks.getY(), grid.templateRows.size())
                && grid.templateRows.getReference(tracks.getY()).isFractional())
            {
                const auto height = juce::jmax(item.height, item.minHeight) + item.margin.top + item.margin.bottom;
                rowFraction = juce::jmax(rowFraction, height / grid.templateRows.getReference(tracks.getY()).getSize());
            }
        }

        for (auto& column : grid.templateColumns)
        {
            if (column.isFractional())
                column = withSize(column, columnFraction * column.getSize());
        }

        for (auto& row : grid.templateRows)
        {
            if (row.isFractional())
                row = withSize(row, rowFraction * row.getSize());
        }
    }

    void GridContainer::layOutChildren()
    {
        if (layoutRecursionLock)
            return;

        JIVE_TRACE();

        const juce::ScopedValueSetter svs{ layoutRecursionLock, true };

        GuiItemDecorator::layOutChildren();

        const auto bounds = boxModel(*this).getContentBounds().toNearestInt();

        if (bounds.isEmpty())
            return;

        do
        {
            changesDuringLayout = false;
            layOutItems(buildGrid(bounds, LayoutStrategy::real), bounds);
        }
        while (changesDuringLayout);
    }

    GridContainer::operator juce::Grid()
    {
        return buildGrid(boxModel(*this).getContentBounds().toNearestInt(),
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

        auto& currentBoxModel = boxModel(*this);

        return {
            extremities.x
                + currentBoxModel
                      .getPadding()
                      .getLeftAndRight()
                + currentBoxModel
                      .getBorder()
                      .getLeftAndRight(),
            extremities.y
                + currentBoxModel
                      .getPadding()
                      .getTopAndBottom()
                + currentBoxModel
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

    void GridContainer::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& id)
    {
        ContainerItem::valueTreePropertyChanged(tree, id);

        if (forwardPropertyChange(tree,
                                  id,
                                  justifyItems,
                                  alignItems,
                                  justifyContent,
                                  alignContent,
                                  gridAutoFlow,
                                  gridTemplateColumns,
                                  gridTemplateRows,
                                  gridTemplateAreas,
                                  gridAutoRows,
                                  gridAutoColumns,
                                  gap))
        {
            return;
        }

        if (tree != state && tree.getParent() != state)
            return;

        if (layoutRecursionLock)
        {
            static const juce::Array<juce::Identifier> propertiesForWhichChangesRequireAnotherLayOut{
                "jive::ideal-width",
                "jive::ideal-height",
            };

            if (propertiesForWhichChangesRequireAnotherLayOut.contains(id))
                changesDuringLayout = true;
        }
    }

    juce::Grid GridContainer::buildGrid(juce::Rectangle<int> bounds,
                                        LayoutStrategy strategy)
    {
        juce::Grid grid;

        grid.autoFlow = gridAutoFlow.getOr(juce::Grid::AutoFlow::row);
        grid.templateColumns = gridTemplateColumns.exists() ? gridTemplateColumns.calculateCurrent() : juce::Array<juce::Grid::TrackInfo>{};
        grid.templateRows = gridTemplateRows.exists() ? gridTemplateRows.calculateCurrent() : juce::Array<juce::Grid::TrackInfo>{};
        grid.templateAreas = gridTemplateAreas.getOr(juce::StringArray{});
        grid.autoRows = gridAutoRows.getOr(juce::Grid::TrackInfo{});
        grid.autoColumns = gridAutoColumns.getOr(juce::Grid::TrackInfo{});

        const auto gaps = gap.exists() ? gap.calculateCurrent() : juce::Array<juce::Grid::Px>{};
        grid.rowGap = gaps.size() > 0 ? gaps.getUnchecked(0) : juce::Grid::Px{ 0 };
        grid.columnGap = gaps.size() > 1 ? gaps.getUnchecked(1) : grid.rowGap;

        appendChildren(*this, grid, bounds, strategy);

        switch (strategy)
        {
        case LayoutStrategy::real:
            grid.justifyItems = justifyItems.getOr(juce::Grid::JustifyItems::stretch);
            grid.alignItems = alignItems.getOr(juce::Grid::AlignItems::stretch);
            grid.justifyContent = justifyContent.getOr(juce::Grid::JustifyContent::stretch);
            grid.alignContent = alignContent.getOr(juce::Grid::AlignContent::stretch);
            break;
        case LayoutStrategy::dummy:
            grid.justifyItems = juce::Grid::JustifyItems::start;
            grid.alignItems = juce::Grid::AlignItems::start;
            grid.justifyContent = juce::Grid::JustifyContent::start;
            grid.alignContent = juce::Grid::AlignContent::start;
            sizeFractionalTracksToContent(grid);

            break;
        }

        return grid;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

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
        testAutoTracks();
        testAlignedItemsKeepIdealSize();
        testAutoSize();
        testFractionalTracksAutoSize();
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
                { "grid-template-columns", "1fr" },
            },
            {
                juce::ValueTree{ "Component" },
                juce::ValueTree{ "Component" },
                juce::ValueTree{ "Component" },
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

        state.setProperty("grid-template-rows", "78px 3fr auto", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 78 },
                           juce::Grid::Fr{ 3 },
                           juce::Grid::TrackInfo{},
                       }));

        state.setProperty("grid-template-rows", "1 313 67", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expect(compare(grid.templateRows,
                       juce::Array<juce::Grid::TrackInfo>{
                           juce::Grid::Px{ 1 },
                           juce::Grid::Px{ 313 },
                           juce::Grid::Px{ 67 },
                       }));
        expectEquals(item->getComponent()->getChildComponent(0)->getHeight(), 1);
        expectEquals(item->getComponent()->getChildComponent(1)->getHeight(), 313);
        expectEquals(item->getComponent()->getChildComponent(2)->getHeight(), 67);

        state.setProperty("grid-template-rows", "1fr 1fr 1fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expectEquals(item->getComponent()->getChildComponent(0)->getHeight(), 111);
        expectEquals(item->getComponent()->getChildComponent(1)->getHeight(), 111);
        expectEquals(item->getComponent()->getChildComponent(2)->getHeight(), 111);

        state.setProperty("grid-template-rows", "3fr 2fr 1fr", nullptr);
        grid = static_cast<juce::Grid>(*dynamic_cast<jive::GuiItemDecorator&>(*item)
                                            .toType<jive::GridContainer>());
        expectWithinAbsoluteError(static_cast<float>(item->getComponent()->getChildComponent(0)->getHeight()), 166.5f, 0.5f);
        expectWithinAbsoluteError(static_cast<float>(item->getComponent()->getChildComponent(1)->getHeight()), 111.0f, 0.5f);
        expectWithinAbsoluteError(static_cast<float>(item->getComponent()->getChildComponent(2)->getHeight()), 55.5f, 0.5f);
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

        state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        state.setProperty("grid-template-columns", "1fr 1fr", nullptr);
        state.setProperty("gap", "0", nullptr);
        const auto& secondChild = *item->getChildren()[1]->getComponent();
        expectEquals(secondChild.getX(), 111);

        state.setProperty("gap", "20", nullptr);
        expectEquals(secondChild.getX(), 121);
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

    void testAutoTracks()
    {
        beginTest("auto tracks");

        const auto idealSized = [](int width, int height) {
            return juce::ValueTree{
                "Component",
                {},
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", width },
                            { "height", height },
                        },
                    },
                },
            };
        };
        juce::ValueTree state{
            "Component",
            {
                { "display", "grid" },
                { "width", 200 },
                { "height", 200 },
                { "grid-template-columns", "auto auto 1fr" },
                { "grid-template-rows", "auto auto" },
            },
            {
                idealSized(30, 10),
                idealSized(50, 20),
                idealSized(10, 10),
                idealSized(20, 15),
                idealSized(40, 10),
                idealSized(10, 10),
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        const auto boundsOfChild = [&item](int index) {
            return item->getChildren()[index]->getComponent()->getBounds();
        };
        expectEquals(boundsOfChild(0), juce::Rectangle{ 0, 0, 30, 20 });
        expectEquals(boundsOfChild(1), juce::Rectangle{ 30, 0, 50, 20 });
        expectEquals(boundsOfChild(2), juce::Rectangle{ 80, 0, 120, 20 });
        expectEquals(boundsOfChild(3), juce::Rectangle{ 0, 20, 30, 15 });
        expectEquals(boundsOfChild(4), juce::Rectangle{ 30, 20, 50, 15 });
        expectEquals(boundsOfChild(5), juce::Rectangle{ 80, 20, 120, 15 });
    }

    void testAlignedItemsKeepIdealSize()
    {
        beginTest("aligned items keep their ideal size");

        const auto idealSized = [](std::initializer_list<juce::NamedValueSet::NamedValue> properties) {
            return juce::ValueTree{
                "Component",
                properties,
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 30 },
                            { "height", 20 },
                        },
                    },
                },
            };
        };
        juce::ValueTree state{
            "Component",
            {
                { "display", "grid" },
                { "width", 300 },
                { "height", 100 },
                { "grid-template-columns", "100px 100px 100px" },
                { "grid-template-rows", "100px" },
                { "align-items", "centre" },
            },
            {
                idealSized({}),
                idealSized({ { "align-self", "end" }, { "justify-self", "start" } }),
                idealSized({ { "align-self", "stretch" }, { "justify-self", "centre" } }),
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(state);
        const auto boundsOfChild = [&item](int index) {
            return item->getChildren()[index]->getComponent()->getBounds();
        };
        expectEquals(boundsOfChild(0), juce::Rectangle{ 0, 40, 100, 20 });
        expectEquals(boundsOfChild(1), juce::Rectangle{ 100, 80, 30, 20 });
        expectEquals(boundsOfChild(2), juce::Rectangle{ 235, 0, 30, 100 });
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

    void testFractionalTracksAutoSize()
    {
        beginTest("auto-size with fractional tracks");

        const auto sized = [](int width, int height) {
            return juce::ValueTree{
                "Component",
                {
                    { "width", width },
                    { "height", height },
                },
            };
        };
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
                        { "grid-template-columns", "1fr 2fr" },
                        { "grid-template-rows", "1fr 1fr" },
                        { "gap", "5" },
                    },
                    {
                        sized(10, 20),
                        sized(30, 10),
                        sized(10, 10),
                        sized(10, 30),
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& item = *parent->getChildren()[0];
        const auto& boxModel = jive::boxModel(item);

        // Each fraction is as big as the biggest content needs it to be, so
        // every track fits its content once the space is shared out.
        expectEquals(boxModel.getWidth(), 15.0f + 5.0f + 30.0f);
        expectEquals(boxModel.getHeight(), 30.0f + 5.0f + 30.0f);

        for (const auto* child : item.getChildren())
            expect(item.getComponent()->getLocalBounds().contains(child->getComponent()->getBounds()));
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
                              .getAttributedString()
                              .getAttribute(0)
                              .font;

        // Both columns are a fraction each, so they're both as wide as the
        // widest text.
        const auto expectedWidth = 2.0f * juce::jmax(std::ceil(jive::calculateStringWidth("hello", font)), std::ceil(jive::calculateStringWidth("world", font)));
        expectEquals(boxModel.getContentBounds().getWidth(), expectedWidth);

        const auto expectedHeight = std::ceil(font.getHeight()) * 2.0f;
        expectEquals(boxModel.getContentBounds().getHeight(), expectedHeight);

        container.state.getChild(0).setProperty("text", "hello world lorum ipsum dolor etc...", nullptr);
        expectEquals(boxModel.getContentBounds().getWidth(), static_cast<float>(state["width"]));

        // Lines aren't always exactly the font's height apart (JUCE 7 adds
        // leading between them), so the expected height comes from a layout.
        const auto& wrappedText = *container.getChildren()[0]->getComponent();
        juce::TextLayout wrappedLayout;
        wrappedLayout.createLayout(dynamic_cast<jive::GuiItemDecorator*>(container.getChildren()[0])
                                       ->toType<jive::Text>()
                                       ->getTextComponent()
                                       .getAttributedString(),
                                   static_cast<float>(wrappedText.getWidth()));
        expectEquals(wrappedLayout.getNumLines(), 2);
        expectEquals(jive::boxModel(*container.getChildren()[0]).getHeight(), std::ceil(wrappedLayout.getHeight()));
    }
};

static GridContainerTest gridContainerTest;
#endif
