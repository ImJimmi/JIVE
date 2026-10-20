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
            boxModel(item)
                .setSize(static_cast<float>(getWidth()),
                         static_cast<float>(getHeight()));
        }

        void moved() final
        {
            item.getComponent()->setTopLeftPosition(getPosition());
        }

    private:
        GuiItem& item;
    };

    FlexItem::FlexItem(std::unique_ptr<GuiItem> itemToDecorate)
        : ContainerItem::Child{ std::move(itemToDecorate) }
        , order{ state, ids::order() }
        , flexGrow{ state, ids::flexGrow() }
        , flexShrink{ state, ids::flexShrink() }
        , flexBasis{ state, ids::flexBasis() }
        , alignSelf{ state, ids::alignSelf() }
        , parentDirection{ state.getParent(), ids::flexDirection() }
        , layoutDummy{ std::make_unique<FlexLayoutDummy>(*this) }
    {
        const auto updateParentLayout = [this] {
            cachedItems.clear();

            if (auto* containerParent = dynamic_cast<GuiItemDecorator&>(*getParent()).getTopLevelDecorator().toType<ContainerItem>())
                containerParent->updateIdealSize();
        };
        order.onValueChange = updateParentLayout;
        flexGrow.onValueChange = updateParentLayout;
        flexGrow.onTransitionProgressed = updateParentLayout;
        flexShrink.onValueChange = updateParentLayout;
        flexShrink.onTransitionProgressed = updateParentLayout;
        flexBasis.onValueChange = updateParentLayout;
        flexBasis.onTransitionProgressed = updateParentLayout;
        alignSelf.onValueChange = updateParentLayout;

        state.addListener(this);
    }

    FlexItem::~FlexItem()
    {
        state.removeListener(this);
    }

    juce::FlexItem FlexItem::toJuceFlexItem(juce::Rectangle<float> parentContentBounds,
                                            LayoutStrategy strategy)
    {
        JIVE_TRACE("x", parentContentBounds.getX(), "y", parentContentBounds.getY(), "width", parentContentBounds.getWidth(), "height", parentContentBounds.getHeight(), "strategy", strategy);
        const auto orientation = [this] {
            const auto direction = parentDirection.getOr(juce::FlexBox::Direction::column);

            if (direction == juce::FlexBox::Direction::row || direction == juce::FlexBox::Direction::rowReverse)
                return Orientation::horizontal;

            return Orientation::vertical;
        }();

        if (box.getConstraintsVersion() != cachedItemsConstraintsVersion
            || orientation != cachedItemsOrientation)
        {
            cachedItems.clear();
            cachedItemsConstraintsVersion = box.getConstraintsVersion();
            cachedItemsOrientation = orientation;
        }

        const auto key = std::make_pair(parentContentBounds, strategy);

        if (cachedItems.find(key) == std::end(cachedItems))
        {
            juce::FlexItem flexItem{ *layoutDummy };

            flexItem.flexShrink = flexShrink.exists() ? flexShrink.calculateCurrent() : 1.0f;

            if (strategy == LayoutStrategy::real)
            {
                flexItem.flexGrow = flexGrow.calculateCurrent();
                flexItem.flexBasis = flexBasis.calculateCurrent();
                flexItem.alignSelf = alignSelf;
            }

            applyConstraints(flexItem,
                             parentContentBounds,
                             orientation,
                             strategy);

            if (std::size(cachedItems) >= maxNumCachedItems)
                cachedItems.clear();

            cachedItems[key] = flexItem;
        }

        lastMinHeight = cachedItems.find(key)->second.minHeight;
        return cachedItems.find(key)->second;
    }

    void FlexItem::idealSizeChanged()
    {
        cachedItems.clear();
    }

    void FlexItem::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        forwardPropertyChange(tree, property, order, flexGrow, flexShrink, flexBasis, alignSelf);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class FlexItemUnitTest : public juce::UnitTest
{
public:
    FlexItemUnitTest()
        : juce::UnitTest{ "jive::FlexItem", "jive" }
    {
    }

    void runTest() final
    {
        testExplicitlySizedItemNotShrunk();
        testItemAfterSiblingThatReflows();
        testConstraintsReusedAfterResize();
        testConstraintsRecalculatedWhenParentDirectionChanges();
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
    void testExplicitlySizedItemNotShrunk()
    {
        beginTest("an item with an explicit size is not shrunk below it when a sibling grows");

        jive::Interpreter interpreter;
        const auto state = jive::parseXML(R"(
            <Component width="600" height="400" align-items="centre" justify-content="centre">
                <Component id="heading" max-width="325" align-items="centre">
                    <Component flex-direction="row" align-items="centre">
                        <svg id="icon" width="25" height="25">
                            <circle cx="50%" cy="50%" r="50%" fill="blue" />
                        </svg>
                        <Text id="title">Short</Text>
                    </Component>
                </Component>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto* icon = root->getChildren()[0]
                               ->getChildren()[0]
                               ->getChildren()[0]
                               ->getComponent()
                               .get();
        expectEquals(icon->getWidth(), 25);
        expectEquals(icon->getHeight(), 25);

        // Growing a sibling so that the row would overflow the max-width should
        // shrink/wrap that sibling, not the fixed-size icon.
        auto titleState = state.getChild(0).getChild(0).getChild(1);
        titleState.setProperty("text",
                               "Live Editing Demo with a great deal more text that definitely wraps onto multiple lines",
                               nullptr);

        expectEquals(icon->getWidth(), 25);
        expectEquals(icon->getHeight(), 25);
    }

    void testItemAfterSiblingThatReflows()
    {
        beginTest("an item is resized when a sibling before it reflows its text");

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component width="1000" height="500">
                <Component>
                    <Text>This sentence is long enough that it has to wrap onto another line when its parent narrows.</Text>
                </Component>
                <Component height="10" />
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto& item = *root->getChildren()[1]->getComponent();
        expectEquals(item.getWidth(), 1000);

        state.setProperty("width", 300, nullptr);
        expectEquals(item.getWidth(), 300);
    }

    void testConstraintsReusedAfterResize()
    {
        beginTest("an item's constraints aren't recalculated when it has only been resized");

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>This sentence is long enough to wrap when its parent narrows.</Text>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        const auto numIdealHeightCalculations = std::make_shared<int>(0);
        auto text = state.getChild(0);
        const auto calculateIdealHeight = text["jive::ideal-height"].getNativeFunction();
        text.setProperty("jive::ideal-height",
                         juce::var{ [numIdealHeightCalculations, calculateIdealHeight](const juce::var::NativeFunctionArgs& args) {
                             ++(*numIdealHeightCalculations);
                             return calculateIdealHeight(args);
                         } },
                         nullptr);

        state.setProperty("width", 200, nullptr);
        expectEquals(root->getChildren()[0]->getComponent()->getWidth(), 200);

        *numIdealHeightCalculations = 0;
        state.setProperty("justify-content", "centre", nullptr);
        expectEquals(*numIdealHeightCalculations, 0);
    }

    void testConstraintsRecalculatedWhenParentDirectionChanges()
    {
        beginTest("an item's constraints are recalculated when its parent's direction changes");

        jive::Interpreter interpreter;
        auto state = jive::parseXML(R"(
            <Component width="300" height="300">
                <Text>This sentence is long enough to wrap when its parent narrows.</Text>
            </Component>
        )");
        auto root = interpreter.interpret(state);
        auto& item = *dynamic_cast<jive::GuiItemDecorator&>(*root->getChildren()[0])
                          .toType<jive::FlexItem>();
        const juce::Rectangle<float> parentContentBounds{ 50.0f, 5.0f };

        auto flexItem = item.toJuceFlexItem(parentContentBounds, jive::LayoutStrategy::real);
        expectGreaterThan(flexItem.minHeight, parentContentBounds.getHeight());

        state.setProperty("flex-direction", "row", nullptr);
        flexItem = item.toJuceFlexItem(parentContentBounds, jive::LayoutStrategy::real);
        expectEquals(flexItem.height, parentContentBounds.getHeight());
    }

    void testOrder()
    {
        beginTest("order");

        jive::Interpreter interpreter;
        const juce::ValueTree state{
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

        expectEquals(flexItem.order, 0);

        state.getChild(0).setProperty("order", 10, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expectEquals(flexItem.order, 10);
    }

    void testFlexGrow()
    {
        beginTest("flex-grow");

        jive::Interpreter interpreter;
        const juce::ValueTree state{
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

        expect(juce::approximatelyEqual(flexItem.flexGrow, 0.0f));

        state.getChild(0).setProperty("flex-grow", 5.0f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(juce::approximatelyEqual(flexItem.flexGrow, 5.0f));
    }

    void testFlexShrink()
    {
        beginTest("flex-shrink");

        jive::Interpreter interpreter;
        const juce::ValueTree state{
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

        expect(juce::approximatelyEqual(flexItem.flexShrink, 1.0f));

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
        const juce::ValueTree state{
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

        expect(juce::approximatelyEqual(flexItem.flexBasis, 0.0f));

        state.getChild(0).setProperty("flex-basis", 4.0f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(juce::approximatelyEqual(flexItem.flexBasis, 4.0f));
    }

    void testAlignSelf()
    {
        beginTest("align-self");

        jive::Interpreter interpreter;
        const juce::ValueTree state{
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
        const juce::ValueTree state{
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

        state.getChild(0).setProperty("width", 50.0f, nullptr);
        state.getChild(0).setProperty("height", 175.0f, nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expectEquals(flexItem.width, 50.0f);
        expectEquals(flexItem.height, 175.0f);
    }

    void testMargin()
    {
        beginTest("margin");

        jive::Interpreter interpreter;
        const juce::ValueTree state{
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

        expect(juce::approximatelyEqual(flexItem.margin.top, 0.0f));
        expect(juce::approximatelyEqual(flexItem.margin.right, 0.0f));
        expect(juce::approximatelyEqual(flexItem.margin.bottom, 0.0f));
        expect(juce::approximatelyEqual(flexItem.margin.left, 0.0f));

        state.getChild(0).setProperty("margin", "1 2 3 4", nullptr);
        flexItem = dynamic_cast<jive::GuiItemDecorator&>(item)
                       .toType<jive::FlexItem>()
                       ->toJuceFlexItem({}, jive::LayoutStrategy::real);

        expect(juce::approximatelyEqual(flexItem.margin.top, 1.0f));
        expect(juce::approximatelyEqual(flexItem.margin.right, 2.0f));
        expect(juce::approximatelyEqual(flexItem.margin.bottom, 3.0f));
        expect(juce::approximatelyEqual(flexItem.margin.left, 4.0f));
    }

    void testRelativeSizes()
    {
        beginTest("relative sizes");

        const juce::ValueTree state{
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
