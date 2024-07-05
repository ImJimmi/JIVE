#include "jive_FlexItem.h"

namespace jive
{
    class FlexLayoutDummy : public juce::Component
    {
    public:
        explicit FlexLayoutDummy(GuiItem& owner)
            : item{ owner }
        {
        }

        void resized() final
        {
            if (strategy == LayoutStrategy::real)
            {
                boxModel(item)
                    .setSize(static_cast<float>(getWidth()),
                             static_cast<float>(getHeight()));
            }
        }

        void moved() final
        {
            if (strategy == LayoutStrategy::real)
                item.getComponent()->setTopLeftPosition(getPosition());
        }

        void setStrategy(LayoutStrategy layoutStrategy)
        {
            strategy = layoutStrategy;
        }

    private:
        LayoutStrategy strategy;
        GuiItem& item;
    };

    FlexItem::FlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem::Child{ std::move(itemToDecorate) }
        , order{ state, "order" }
        , flexGrow{ state, "flex-grow" }
        , flexShrink{ state, "flex-shrink" }
        , flexBasis{ state, "flex-basis" }
        , alignSelf{ state, "align-self" }
        , layoutDummy{ std::make_unique<FlexLayoutDummy>(*this) }
    {
        if (!flexShrink.exists())
            flexShrink = juce::FlexItem{}.flexShrink;

        const auto updateParentLayout = [this]() {
            cachedItems.clear();
            getParent()->callLayoutChildrenWithRecursionLock();
        };
        order.onValueChange = updateParentLayout;
        flexGrow.onValueChange = updateParentLayout;
        flexGrow.onTransitionProgressed = updateParentLayout;
        flexShrink.onValueChange = updateParentLayout;
        flexShrink.onTransitionProgressed = updateParentLayout;
        flexBasis.onValueChange = updateParentLayout;
        flexBasis.onTransitionProgressed = updateParentLayout;
        alignSelf.onValueChange = updateParentLayout;

        box.addListener(*this);
    }

    FlexItem::~FlexItem()
    {
        box.removeListener(*this);
    }

    juce::FlexItem FlexItem::toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                            LayoutStrategy strategy)
    {
        const auto key = std::make_pair(parentContentBounds, strategy);

        if (cachedItems.find(key) == std::end(cachedItems))
        {
            juce::FlexItem flexItem{ *layoutDummy };

            flexItem.flexShrink = flexShrink.calculateCurrent();

            if (strategy == LayoutStrategy::real)
            {
                flexItem.flexGrow = flexGrow.calculateCurrent();
                flexItem.flexBasis = flexBasis.calculateCurrent();
                flexItem.alignSelf = alignSelf;
            }

            const auto orientation = [this]() {
                const Property<juce::FlexBox::Direction> parentDirection{
                    state.getParent(),
                    "flex-direction",
                };
                const auto direction = parentDirection.get();

                if (direction == juce::FlexBox::Direction::row || direction == juce::FlexBox::Direction::rowReverse)
                    return Orientation::horizontal;

                return Orientation::vertical;
            };
            applyConstraints(flexItem,
                             parentContentBounds,
                             orientation(),
                             strategy);

            cachedItems[key] = flexItem;
        }

        dynamic_cast<FlexLayoutDummy&>(*layoutDummy).setStrategy(strategy);
        return cachedItems.find(key)->second;
    }

    void FlexItem::boxModelChanged(BoxModel&)
    {
        cachedItems.clear();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

class FlexItemUnitTest : public juce::UnitTest
{
public:
    FlexItemUnitTest()
        : juce::UnitTest{ "jive::FlexItem", "jive" }
    {
    }

    void runTest() final
    {
        testOrder();
        testFlexGrow();
        testFlexShrink();
        testFlexBasis();
        testAlignSelf();
        testSize();
        testMargin();
        testRelativeSizes();
    }

private:
    void testOrder()
    {
        beginTest("order");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.order == 0);

        state.getChild(0).setProperty("order", 10, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.order == 10);
    }

    void testFlexGrow()
    {
        beginTest("flex-grow");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexGrow == 0.f);

        state.getChild(0).setProperty("flex-grow", 5.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexGrow == 5.f);
    }

    void testFlexShrink()
    {
        beginTest("flex-shrink");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexShrink == 1.f);

        state.getChild(0).setProperty("flex-shrink", 3.4f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expectEquals(flexItem.flexShrink, 3.4f);
    }

    void testFlexBasis()
    {
        beginTest("flex-basis");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexBasis == 0.f);

        state.getChild(0).setProperty("flex-basis", 4.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.flexBasis == 4.f);
    }

    void testAlignSelf()
    {
        beginTest("align-self");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.alignSelf == juce::FlexItem{}.alignSelf);

        state.getChild(0).setProperty("align-self", "centre", nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.alignSelf == juce::FlexItem::AlignSelf::center);
    }

    void testSize()
    {
        beginTest("size");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 200 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);
        expectEquals(flexItem.width, juce::FlexItem{}.width);
        expectEquals(flexItem.height, juce::FlexItem{}.height);

        state.getChild(0).setProperty("width", 50.f, nullptr);
        state.getChild(0).setProperty("height", 175.f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expectEquals(flexItem.width, 50.f);
        expectEquals(flexItem.height, 175.f);
    }

    void testMargin()
    {
        beginTest("margin");

        jive::Interpreter interpreter;
        juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 200 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        };
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        auto flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                            .toType<jive::FlexItem>()
                            ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.margin.top == 0.f);
        expect(flexItem.margin.right == 0.f);
        expect(flexItem.margin.bottom == 0.f);
        expect(flexItem.margin.left == 0.f);

        state.getChild(0).setProperty("margin", "1 2 3 4", nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(flexItem.margin.top == 1.f);
        expect(flexItem.margin.right == 2.f);
        expect(flexItem.margin.bottom == 3.f);
        expect(flexItem.margin.left == 4.f);
    }

    void testRelativeSizes()
    {
        beginTest("relative sizes");

        juce::ValueTree state{
            "Component",
            {
                { "width", 300 },
                { "height", 200 },
                { "display", "flex" },
                { "align-items", "flex-start" },
            },
            {
                juce::ValueTree{
                    "Component",
                    {
                        { "width", "50%" },
                        { "height", "10%" },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(state);
        auto& item = *parent->getChildren()[0];
        const auto& boxModel = jive::boxModel(item);
        expectEquals(boxModel.getWidth(), 150.0f);
        expectEquals(boxModel.getHeight(), 20.0f);
    }
};

static FlexItemUnitTest flexItemUnitTest;
#endif
