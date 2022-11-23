#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GridItem::GridItem(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , order{ state, "order", juce::GridItem{}.order }
        , justifySelf{ state, "justify-self", juce::GridItem{}.justifySelf }
        , alignSelf{ state, "align-self", juce::GridItem{}.alignSelf }
        , column{ state, "column", juce::GridItem{}.column }
        , row{ state, "row", juce::GridItem{}.row }
        , area{ state, "area", juce::GridItem{}.area }
        , maxWidth{ state, "max-width", juce::GridItem{}.maxWidth }
        , maxHeight{ state, "max-height", juce::GridItem{}.maxHeight }
        , width{ state, "width", "auto" }
        , height{ state, "height", "auto" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , autoMinWidth{ state, "auto-min-width" }
        , autoMinHeight{ state, "auto-min-height" }
    {
        jassert(getParent() != nullptr);

        const auto updateParentLayout = [this]() {
            getParent()->layOutChildren();
        };
        order.onValueChange = updateParentLayout;
        justifySelf.onValueChange = updateParentLayout;
        alignSelf.onValueChange = updateParentLayout;
        column.onValueChange = updateParentLayout;
        row.onValueChange = updateParentLayout;
        area.onValueChange = updateParentLayout;
        minWidth.onValueChange = updateParentLayout;
        maxWidth.onValueChange = updateParentLayout;
        minHeight.onValueChange = updateParentLayout;
        maxHeight.onValueChange = updateParentLayout;
        width.onValueChange = updateParentLayout;
        height.onValueChange = updateParentLayout;
    }

    //==================================================================================================================
    juce::GridItem::Margin boxModelToGridItemMargin(const BoxModel& boxModel)
    {
        const auto margin = boxModel.getMargin();

        return {
            margin.getTop(),
            margin.getRight(),
            margin.getBottom(),
            margin.getLeft(),
        };
    }

    GridItem::operator juce::GridItem()
    {
        juce::GridItem gridItem{ *component };
        const auto parentBounds = getParent()->boxModel.getBounds();

        if (!width.isAuto())
            gridItem.width = width.toPixels(parentBounds);
        if (!height.isAuto())
            gridItem.height = height.toPixels(parentBounds);

        const auto minBounds = boxModel.getMinimumBounds();
        gridItem.minWidth = minBounds.getWidth();
        gridItem.minHeight = minBounds.getHeight();

        gridItem.maxWidth = maxWidth;
        gridItem.maxHeight = maxHeight;

        gridItem.order = order;

        gridItem.justifySelf = justifySelf;
        gridItem.alignSelf = alignSelf;

        gridItem.column = column;
        gridItem.row = row;
        gridItem.area = area;

        gridItem.margin = boxModelToGridItemMargin(boxModel);

        return gridItem;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
bool compare(juce::GridItem::Property a, juce::GridItem::Property b)
{
    return a.getName() == b.getName() && a.getNumber() == b.getNumber();
}

bool compare(juce::GridItem::StartAndEndProperty a,
             juce::GridItem::StartAndEndProperty b)
{
    return compare(a.start, b.start) && compare(a.end, b.end);
}

bool compare(juce::GridItem::Margin a, juce::GridItem::Margin b)
{
    return a.top == b.top && a.right == b.right && a.bottom == b.bottom && a.left == b.left;
}

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
        testJustifySelf();
        testAlignSelf();
        testColumn();
        testRow();
        testArea();
        testWidth();
        testMinWidth();
        testMaxWidth();
        testHeight();
        testMinHeight();
        testMaxHeight();
        testMargin();
    }

private:
    void testComponent()
    {
        beginTest("component");

        jive::Interpreter interpreter;
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
        auto parent = interpreter.interpret(state);
        auto& item = parent->getChild(0);
        const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                               .toType<jive::GridItem>());
        expect(gridItem.associatedComponent == item.getComponent().get());
    }

    void testOrder()
    {
        beginTest("order");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expectEquals(gridItem.order, 0);

            state.getChild(0).setProperty("order", 123, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expectEquals(gridItem.order, 123);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "order", 456 },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expectEquals(gridItem.order, 456);
        }
    }

    void testJustifySelf()
    {
        beginTest("justify-self");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::autoValue);

            state.getChild(0).setProperty("justify-self", "start", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::start);

            state.getChild(0).setProperty("justify-self", "end", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::end);

            state.getChild(0).setProperty("justify-self", "centre", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::center);

            state.getChild(0).setProperty("justify-self", "stretch", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::stretch);

            state.getChild(0).setProperty("justify-self", "auto", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::autoValue);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "justify-self", "end" },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.justifySelf == juce::GridItem::JustifySelf::end);
        }
    }

    void testAlignSelf()
    {
        beginTest("align-self");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::autoValue);

            state.getChild(0).setProperty("align-self", "start", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::start);

            state.getChild(0).setProperty("align-self", "end", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::end);

            state.getChild(0).setProperty("align-self", "centre", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::center);

            state.getChild(0).setProperty("align-self", "stretch", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::stretch);

            state.getChild(0).setProperty("align-self", "auto", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::autoValue);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "align-self", "end" },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.alignSelf == juce::GridItem::AlignSelf::end);
        }
    }

    void testColumn()
    {
        beginTest("column");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(compare(gridItem.column, juce::GridItem::StartAndEndProperty{}));

            state.getChild(0).setProperty("column", "3 / span 4", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(compare(gridItem.column, juce::GridItem::StartAndEndProperty{ 3, juce::GridItem::Span{ 4 } }));
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                    { "template-columns", "auto auto auto auto" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "column", "1 / span 3" },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(compare(gridItem.column, juce::GridItem::StartAndEndProperty{ 1, juce::GridItem::Span{ 3 } }));
        }
    }

    void testRow()
    {
        beginTest("row");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(compare(gridItem.row, juce::GridItem::StartAndEndProperty{}));

            state.getChild(0).setProperty("row", "2 / 3", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(compare(gridItem.row, juce::GridItem::StartAndEndProperty{ 2, 3 }));
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "row", "14 / span 7" },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(compare(gridItem.row, juce::GridItem::StartAndEndProperty{ 14, juce::GridItem::Span{ 7 } }));
        }
    }

    void testArea()
    {
        beginTest("area");

        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                    { "template-areas", "just-here" },
                },
                {
                    juce::ValueTree{ "Component" },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.area.isEmpty());

            state.getChild(0).setProperty("area", "just-here", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.area == "just-here");
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                    { "template-areas", "xyz abc" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "area", "abc" },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.area == "abc");
        }
    }

    void testWidth()
    {
        beginTest("width");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expectEquals<float>(gridItem.width, juce::GridItem::notAssigned);

            state.getChild(0).setProperty("width", 112.f, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expectEquals(gridItem.width, 112.f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "width", 374.6f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expectEquals(gridItem.width, 374.6f);
        }
    }

    void testMinWidth()
    {
        beginTest("min-width");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.minWidth == 0.f);

            state.getChild(0).setProperty("min-width", 493.6f, nullptr);

            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.minWidth == 493.6f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "min-width", 12.6f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.minWidth == 12.6f);
        }
    }

    void testMaxWidth()
    {
        beginTest("max-width");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.maxWidth == juce::GridItem::notAssigned);

            state.getChild(0).setProperty("max-width", 30.4f, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.maxWidth == 30.4f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                    { "justify-self", "end" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "max-width", 986.f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.maxWidth == 986.f);
        }
    }

    void testHeight()
    {
        beginTest("height");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.height == juce::GridItem::notAssigned);

            state.getChild(0).setProperty("height", 112.f, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.height == 112.f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "height", 374.6f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expectEquals(gridItem.height, 374.6f);
        }
    }

    void testMinHeight()
    {
        beginTest("min-height");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.minHeight == 0.f);

            state.getChild(0).setProperty("min-height", 493.6f, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.minHeight == 493.6f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "min-height", 12.6f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.minHeight == 12.6f);
        }
    }

    void testMaxHeight()
    {
        beginTest("max-height");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(gridItem.maxHeight == juce::GridItem::notAssigned);

            state.getChild(0).setProperty("max-height", 30.4f, nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(gridItem.maxHeight == 30.4f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "max-height", 986.f },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(gridItem.maxHeight == 986.f);
        }
    }

    void testMargin()
    {
        beginTest("margin");

        {
            jive::Interpreter interpreter;
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
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                             .toType<jive::GridItem>());
            expect(compare(gridItem.margin, juce::GridItem::Margin{}));

            state.getChild(0).setProperty("margin", "10 20 4 13.67", nullptr);
            gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                        .toType<jive::GridItem>());
            expect(compare(gridItem.margin, juce::GridItem::Margin{ 10.f, 20.f, 4.f, 13.67f }));
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree state{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "display", "grid" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "margin", 45 },
                        },
                    },
                },
            };
            auto parent = interpreter.interpret(state);
            auto& item = parent->getChild(0);
            const auto gridItem = static_cast<juce::GridItem>(*dynamic_cast<jive::GuiItemDecorator&>(item)
                                                                   .toType<jive::GridItem>());
            expect(compare(gridItem.margin, juce::GridItem::Margin{ 45.f }));
        }
    }
};

static GridItemUnitTest gridItemUnitTest;
#endif
